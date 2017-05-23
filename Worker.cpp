#include "Worker.h"

Worker::Worker()
{
	fd = shm_open("tuple_space", O_RDWR, 0777);
	read_meta_data(fd);
	void* mem = mmap(0, shared_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	memory_addr = (int*)mem;
}

void Worker::read_meta_data(int fd)
{
	void* addr = mmap(0, META_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	Meta_Data* meta = (Meta_Data*)addr;
	shared_size = meta->shared_size;
	tuple_array_offset = meta->tuple_array_offset;
	string_array_offset = meta->string_array_offset;
	waiting_array_offset = meta->waiting_array_offset;
	munmap(addr, META_SIZE);
}

Worker::~Worker()
{
	munmap((void*)memory_addr, shared_size);
	close(fd);
}


void Worker::output(Tuple tuple)
{
	lock_semaphore();
	add_tuple_to_memory(tuple);
	Pattern_Pair* waiting_addr = check_waiting_queue(tuple);
	if(waiting_addr != nullptr)
		wake_up_process(waiting_addr);
	unlock_semaphore();
}

Tuple Worker::read(std::string pattern)
{
	lock_semaphore();
	Tuple* tuple_addr = nullptr;
	while(tuple_addr == nullptr)
	{
		tuple_addr = find_tuple_in_memory(pattern);
		if(tuple_addr == nullptr)
			wait_in_memory_for_tuple(pattern);
	}
	Tuple tuple = read_tuple_from_memory(tuple_addr);
	unlock_semaphore();
	return tuple;
}

Tuple Worker::input(std::string pattern)
{
	lock_semaphore();
	Tuple* tuple_addr = nullptr;
	while(tuple_addr == nullptr)
	{
		tuple_addr = find_tuple_in_memory(pattern);
		if(tuple_addr == nullptr)
			wait_in_memory_for_tuple(pattern);
	}
	Tuple tuple = remove_tuple_from_memory(tuple_addr);
	unlock_semaphore();
	return tuple;
}

//put tuple in shared memo.   1 - success, 0 - failure
int Worker::add_tuple_to_memory(Tuple tuple)
{
	Tuple* addr = (Tuple*)(memory_addr + tuple_array_offset);
	while((int*)addr < memory_addr + string_array_offset)
	{
	// TODO: possibly remove this loop and do *only* tuple memcpy (parsing strings in a different method)
		if(addr->data[0].type != NO_DATA)
		{
			for(int i = 0; i < 8; ++i)
			{
				if(tuple.data[i].type == DATA_STRING)
					if((addr->data[i].data_union.data_string =\
					    add_string_to_memory(tuple.data[i].data_union.data_string)))
							addr->data[i].type = tuple.data[i].type;
					else
					{
						remove_tuple_from_memory(addr);
						return 0;
					}
				else
					addr->data[i] = tuple.data[i];
			}
			return 1;
		}
		else
			addr += TUPLE_SIZE;
	}
	return 0;
}

//returns the address of found free space (or nullptr if none found)
char* Worker::add_string_to_memory(const char * user_tuple_str)
{
	char * str_addr = (char*)(memory_addr + string_array_offset);
	while((int*)str_addr < memory_addr + waiting_array_offset)
	{
		if(*str_addr == NULL_SIGN)
		{
			strcpy(str_addr, user_tuple_str);
			return str_addr;
		}
		else
			str_addr += STRING_SIZE;
	}
	return nullptr;
}

//return addr of waiting process or nullptr is noone is waiting
Pattern_Pair* Worker::check_waiting_queue(Tuple tuple)
{
	Pattern_Pair* addr = (Pattern_Pair*)(memory_addr + waiting_array_offset);
	while ((int*)addr < memory_addr + shared_size)
	{
		if (*(addr->pattern) != NULL_SIGN)
			if(compare_tuple_with_pattern(tuple, addr->pattern))
				return addr;
		addr += sizeof(Pattern_Pair);
	}
	return nullptr;
}

//return addr of first matching to pattern tuple, nullptr if nothing is matching 
Tuple* Worker::find_tuple_in_memory(std::string pattern)
{
	Tuple* addr = (Tuple*)(memory_addr + tuple_array_offset);
	while((int*)addr < memory_addr + string_array_offset)
	{
		if(addr->data[0].type != NO_DATA)
			if(compare_tuple_with_pattern(*addr, pattern))
				return addr;
		addr += TUPLE_SIZE;
	}
	return nullptr;
}

//sem_post on semaphore at given address - wakes process waiting for tuple
void Worker::wake_up_process(Pattern_Pair* waiting_addr)
{
	sem_post(waiting_addr->sem);
}

//stores pattern, creates semaphore in memory and waits on it until woken. 
//Clean memory after use
int Worker::wait_in_memory_for_tuple(std::string pattern)
{
	Pattern_Pair* addr = (Pattern_Pair*)(memory_addr + waiting_array_offset);
	while ((int*)addr < memory_addr + shared_size)
	{
		if (*(addr->pattern) == NULL_SIGN)
		{
			strcpy(addr->pattern, pattern.c_str());
			sem_init(addr->sem,1,0);
			unlock_semaphore();
			sem_wait(addr->sem);
			sem_destroy(addr->sem);
			*(addr->pattern) = NULL_SIGN;
			lock_semaphore();
			return 1;
		}
		else
			addr += sizeof(Pattern_Pair);
	}
	return 0;
}

//returns tuple stored at given address
Tuple Worker::read_tuple_from_memory(Tuple* addr)
{
	Tuple tuple = *addr;
	return tuple;
}

//removes and returns tuple at given address
Tuple Worker::remove_tuple_from_memory(Tuple* addr)
{
	Tuple tuple = *addr;
	for(int i = 0; i < 8; ++i)
	{
		if(tuple.data[i].type == DATA_STRING)
		{
			char * str_addr = tuple.data[i].data_union.data_string;
			*str_addr = NULL_SIGN;
		}
		tuple.data[i].type = NO_DATA;
	}
	return tuple;
}

void Worker::lock_semaphore()
{
	Meta_Data* meta = (Meta_Data*)memory_addr;
	sem_wait(meta->main_sem);
}

void Worker::unlock_semaphore()
{
	Meta_Data* meta = (Meta_Data*)memory_addr;
	sem_post(meta->main_sem);
}

bool Worker::compare_tuple_with_pattern(Tuple tuple, std::string str){
    std::stringstream test(str);
    std::string segment;
    std::vector<std::string> elements;

    while(std::getline(test, segment, ','))
    {
       elements.push_back(segment);
    }

    if(elements.size()>8){
        return false;
    }

    for(unsigned i=0;i<elements.size();i++){
        char type = elements[i][0];
        if(type == 's'){
            std::string value = elements[i].substr(2);
            if(value == "*"){
                continue;
            }
            value = value.substr(1);
            value = value.substr(0,value.size()-1);
	#if DEBUG
            std::cout<<value;
	#endif
            if( value  != tuple.data[i].data_union.data_string){
                return false;
            }
        }
        else if(type == 'i' && tuple.data[i].type == DATA_INT){
            if(isdigit(elements[i][2])){
                std::string value = elements[i].substr(2);
                if( (std::stoi( value )) != tuple.data[i].data_union.data_int){
                    return false;
                }
            }
            else{
                std::string value = elements[i].substr(3);
                char sign = elements[i][2];
                if(sign == '*'){
                    continue;
                }
                else if(sign == '>'){
                    if( (std::stoi( value )) >= tuple.data[i].data_union.data_int){
                        return false;
                    }
                }
                else if(sign == '<'){
                    if( (std::stoi( value )) <= tuple.data[i].data_union.data_int){
                        return false;
                    }
                }
                else{
                    return false;
                }
            }

        }
        else if(type == 'f' && tuple.data[i].type == DATA_FLOAT){
            if(isdigit(elements[i][2])){
                std::string value = elements[i].substr(2);
                if( std::stof( value ) != tuple.data[i].data_union.data_float){
                    return false;
                }
            }
            else{
                std::string value = elements[i].substr(3);
                char sign = elements[i][2];
                if(sign == '*'){
                    continue;
                }
                else if(sign == '>'){
                    if( (std::stof( value )) >= tuple.data[i].data_union.data_float){
                        return false;
                    }
                }
                else if(sign == '<'){
                    if( (std::stof( value )) <= tuple.data[i].data_union.data_float){
                        return false;
                    }
                }
                else{
                    return false;
                }
            }
        }
        else{
            return false;
        }
    }
   for(int i = elements.size();i<8;i++){
       if(tuple.data[i].type != NO_DATA){
           return false;
       }
   }
    return true;
}

std::string Worker::get_pattern_from_tuple(Tuple tuple)
{
	std::string pattern;
	int tuple_it = 0;
	while(tuple.data[tuple_it].type != data_type::NO_DATA)
	{
		switch(tuple.data[tuple_it].type)
		{
			case data_type::DATA_INT:
			pattern += "i:";
			pattern += std::to_string(tuple.data[tuple_it].data_union.data_int);
			break;
			case data_type::DATA_FLOAT:
			pattern += "f:";
			pattern += std::to_string(tuple.data[tuple_it].data_union.data_float);
			break;			
			case data_type::DATA_STRING:
			pattern += "s:";
			pattern += tuple.data[tuple_it].data_union.data_string;
			break;
			default:
			pattern += "????";
		}
		tuple_it++;
		if(tuple.data[tuple_it].type != data_type::NO_DATA)
			pattern += ','; 
	}
	return pattern;
}
// it's not really >>pattern<< but I dunno how to call it '_>'
Tuple Worker::convert_pattern_to_tuple(std::string pattern)
{
    std::stringstream temp(pattern);
    std::string segment;
    std::vector<std::string> elements;
    Tuple to_return;

    while(std::getline(temp, segment, ','))
    {
       elements.push_back(segment);
    }
    
    for(unsigned it = 0; it != elements.size(); ++it)
    {
        std::string bit = elements[it];
        std::string substr = bit.substr(2);
        char type = bit[0];
        switch(type)
        {
            case 's':
                to_return.data[it].type = data_type::DATA_STRING;
                to_return.data[it].data_union.data_string = add_string_to_memory(substr.c_str());
                break;
            case 'i':
                to_return.data[it].type = data_type::DATA_INT;
                to_return.data[it].data_union.data_int = std::stoi(substr);
                break;
            case 'f':
                to_return.data[it].type = data_type::DATA_FLOAT;
                to_return.data[it].data_union.data_int = std::stof(substr);
                break;
            default:
                // error parsing string - stop parsing
                to_return.data[it].type = data_type::NO_DATA;
                return to_return;
        }
    }
    return to_return;
}

