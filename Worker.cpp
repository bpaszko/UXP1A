#include "Worker.h"
#include <exception>

Worker::Worker()
{
	fd = shm_open("/tuple_space", O_RDWR, 0777);
	read_meta_data(fd);
	void* mem = mmap(0, shared_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	memory_addr = (char*)mem;
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
	int res = add_tuple_to_memory(tuple);
	if (!res){
		unlock_semaphore();
		exit(-1);
	}
	Pattern_Pair* waiting_addr = check_waiting_queue(tuple);
	if(waiting_addr)
		wake_up_process(waiting_addr);
	unlock_semaphore();
}

Tuple Worker::read(std::string pattern)
{
	lock_semaphore();
	Tuple* tuple_addr = nullptr;
	while(!tuple_addr)
	{
		tuple_addr = find_tuple_in_memory(pattern);
		if(!tuple_addr){
			int res = wait_in_memory_for_tuple(pattern);
			if (!res){
				unlock_semaphore();
				exit(-1);
			}
		}
	}
	Tuple tuple = read_tuple_from_memory(tuple_addr);
	unlock_semaphore();
	return tuple;
}

Tuple Worker::input(std::string pattern)
{
	lock_semaphore();
	Tuple* tuple_addr = nullptr;
	while(!tuple_addr)
	{
		tuple_addr = find_tuple_in_memory(pattern);
		if(!tuple_addr){
			int res = wait_in_memory_for_tuple(pattern);
			if (!res){
				unlock_semaphore();
				exit(-1);
			}
		}
	}
	Tuple tuple = remove_tuple_from_memory(tuple_addr);
	unlock_semaphore();
	return tuple;
}

//put tuple in shared memo.   1 - success, 0 - failure
int Worker::add_tuple_to_memory(Tuple tuple)
{
	Tuple* addr = (Tuple*)(memory_addr + tuple_array_offset);
	while((char*)addr < memory_addr + string_array_offset)
	{
		if(addr->data[0].type == data_type::NO_DATA)
		{
			for(int i = 0; i < 8; ++i)
			{
				if(tuple.data[i].type == data_type::DATA_STRING)
				{
				    char* str_addr = add_string_to_memory(tuple.data[i].data_union.data_string);
					addr->data[i].type = tuple.data[i].type;
					addr->data[i].data_union.data_string = str_addr - long(memory_addr);
				}
				else
					addr->data[i] = tuple.data[i];
			}
			return 1;
		}
		else
			//addr = (Tuple*)((char*)addr + TUPLE_SIZE);
			addr += 1;
	}
	return 0;
}

//returns the address of found free space (or nullptr if none found)
char* Worker::add_string_to_memory(const char * user_tuple_str)
{
	char * str_addr = (char*)(memory_addr + string_array_offset);
	while(str_addr < memory_addr + waiting_array_offset)
	{
		if(*str_addr == NULL_SIGN)
		{
			strcpy(str_addr, user_tuple_str);
			return str_addr;
		}
		else
			str_addr = str_addr + STRING_SIZE;
	}
	return nullptr;
}

//return addr of waiting process or nullptr is noone is waiting
Pattern_Pair* Worker::check_waiting_queue(Tuple tuple)
{
	Pattern_Pair* addr = (Pattern_Pair*)(memory_addr + waiting_array_offset);
	while ((char*)addr < memory_addr + shared_size)
	{
		if (*(addr->pattern) != NULL_SIGN)
			if(compare_tuple_with_pattern(tuple, addr->pattern))
				return addr;
		//addr = (Pattern_Pair*)((char*)addr + sizeof(Pattern_Pair)); 
		addr += 1;
	}
	return nullptr;
}

//return addr of first matching to pattern tuple, nullptr if nothing is matching 
Tuple* Worker::find_tuple_in_memory(std::string pattern)
{
	Tuple* addr = (Tuple*)(memory_addr + tuple_array_offset);
	while((char*)addr < memory_addr + string_array_offset)
	{
		if(addr->data[0].type != data_type::NO_DATA)
			if(compare_tuple_with_pattern(*addr, pattern, long(memory_addr)))
				return addr;
		//addr = (Tuple*)((char*)addr + TUPLE_SIZE);
		addr += 1;
	}
	return nullptr;
}

//sem_post on semaphore at given address - wakes process waiting for tuple
void Worker::wake_up_process(Pattern_Pair* waiting_addr)
{
	sem_post(&(waiting_addr->sem));
}

//stores pattern, creates semaphore in memory and waits on it until woken. 
//Clean memory after use
int Worker::wait_in_memory_for_tuple(std::string pattern)
{
	Pattern_Pair* addr = (Pattern_Pair*)(memory_addr + waiting_array_offset);
	while ((char*)addr < memory_addr + shared_size)
	{
		if (*(addr->pattern) == NULL_SIGN)
		{
			strcpy(addr->pattern, pattern.c_str());
			sem_init(&(addr->sem),1,0);
			unlock_semaphore();
			sem_wait(&(addr->sem));
			sem_destroy(&(addr->sem));
			*(addr->pattern) = NULL_SIGN;
			lock_semaphore();
			return 1;
		}
		else
			//addr = (Pattern_Pair*)((char*)addr + sizeof(Pattern_Pair)); 
			addr += 1;
	}
	return 0;
}

//returns tuple stored at given address
Tuple Worker::read_tuple_from_memory(Tuple* addr)
{
	Tuple tuple;
	for(int i =0; i < 8; ++i){
		switch(addr->data[i].type){
			case data_type::NO_DATA:
				return tuple;
			case data_type::DATA_INT:
			case data_type::DATA_FLOAT:
				tuple.data[i] = addr->data[i];
				break;
			case data_type::DATA_STRING:
				char * str = addr->data[i].data_union.data_string + long(memory_addr);
				tuple.data[i].type = data_type::DATA_STRING;
				tuple.data[i].data_union.data_string = new char[strlen(str)+1];
				strcpy(tuple.data[i].data_union.data_string, str);
				break;
		}
	}
	return tuple;

}

//removes and returns tuple at given address
Tuple Worker::remove_tuple_from_memory(Tuple* addr)
{
	Tuple tuple = read_tuple_from_memory(addr);
	for(int i = 0; i < 8; ++i)
	{
		if(addr->data[i].type == data_type::DATA_STRING)
		{
			char * str_addr = addr->data[i].data_union.data_string + long(memory_addr);
			*str_addr = NULL_SIGN;
		}
		addr->data[i].type = data_type::NO_DATA;
	}
	return tuple;
}

void Worker::lock_semaphore()
{
	Meta_Data* meta = (Meta_Data*)memory_addr;
	sem_wait(&(meta->main_sem));
}

void Worker::unlock_semaphore()
{
	Meta_Data* meta = (Meta_Data*)memory_addr;
	sem_post(&(meta->main_sem));
}


bool Worker::compare_tuple_with_pattern(const Tuple& tuple, std::string str, long mem_offset){
    std::stringstream test(str);
    std::string segment;
    std::vector<std::string> elements;
    while(std::getline(test, segment, ','))
       elements.push_back(segment);

    if(elements.size()>8)
        throw std::exception();

    for(unsigned i=0;i<elements.size();i++){
        char type = elements[i][0];
        switch(type){
        	case 'f':
        		if(!compare_float(elements[i], tuple.data[i]))
        			return false;
        		break;
        	case 'i':
        		if(!compare_int(elements[i], tuple.data[i]))
        			return false;
        		break;
        	case 's':
        		if(!compare_string(elements[i], tuple.data[i], mem_offset))
        			return false;
        		break;
        	default:
        		throw std::exception();
        }
    }
    for(int i = elements.size();i<8;i++)
        if(tuple.data[i].type != data_type::NO_DATA)
            return false;
    return true;
}


bool Worker::compare_string(const std::string& pattern, const Tuple_Data& data, long mem_offset){
	if(data.type != data_type::DATA_STRING)
		return false;
	std::string value = pattern.substr(2);
	switch(value[0]){
		case '*':
			return true;
		case '>':
			if(value[1] == '='){
				value = value.substr(3, value.size()-4);
				if(value.size()+1 > 64)
	            	throw std::exception();
	            return (data.data_union.data_string + mem_offset) >= value;
			}
			value = value.substr(2, value.size()-3);
			if(value.size()+1 > 64)
            	throw std::exception();
            return (data.data_union.data_string + mem_offset) > value;
		case '<':
			if(value[1] == '='){
				value = value.substr(3, value.size()-4);
				if(value.size()+1 > 64)
	            	throw std::exception();
	            return (data.data_union.data_string + mem_offset) <= value;
			}
			value = value.substr(2, value.size()-3);
			if(value.size()+1 > 64)
            	throw std::exception();
            return (data.data_union.data_string + mem_offset) < value;
		case '"':
			value = value.substr(1, value.size()-2);
			if(value.size()+1 > 64)
            	throw std::exception();
            return (data.data_union.data_string + mem_offset) == value;
		default:
			throw std::exception();
	}


}

bool Worker::compare_int(const std::string& pattern, const Tuple_Data& data){
	if(data.type != data_type::DATA_INT)
		return false;
	std::string value = pattern.substr(2);
	switch(value[0]){
		case '*':
			return true;
		case '>':
			if(value[1] == '='){
				value = value.substr(2);
	            return data.data_union.data_int >= std::stoi(value);
			}
			value = value.substr(1);
            return data.data_union.data_int > std::stoi(value);
		case '<':
			if(value[1] == '='){
				value = value.substr(2);
	            return data.data_union.data_int <= std::stoi(value);
			}
			value = value.substr(1);
            return data.data_union.data_int < std::stoi(value);
		default:
            return data.data_union.data_int == std::stoi(value);
	}

}

bool Worker::compare_float(const std::string& pattern, const Tuple_Data& data){
	if(data.type != data_type::DATA_FLOAT)
		return false;
	std::string value = pattern.substr(2);
	switch(value[0]){
		case '*':
			return true;
		case '>':
			if(value[1] == '='){
				value = value.substr(2);
	            return data.data_union.data_float >= std::stof(value);
			}
			value = value.substr(1);
            return data.data_union.data_float > std::stof(value);
		case '<':
			if(value[1] == '='){
				value = value.substr(2);
	            return data.data_union.data_float <= std::stof(value);
			}
			value = value.substr(1);
            return data.data_union.data_float < std::stof(value);
		default:
            throw std::exception();
        }
}


/*bool Worker::compare_tuple_with_pattern(const Tuple& tuple, std::string str, long mem_offset){
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
        if(type == 's' && tuple.data[i].type == data_type::DATA_STRING){
            std::string value = elements[i].substr(2);
            if(value == "*"){
                continue;
            }
            value = value.substr(1);
            value = value.substr(0,value.size()-1);
            if( value.size()+1 > 64)
            	throw std::exception();
            if( value  != tuple.data[i].data_union.data_string + mem_offset){
                return false;
            }
        }
        else if(type == 'i' && tuple.data[i].type == data_type::DATA_INT){
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
        else if(type == 'f' && tuple.data[i].type == data_type::DATA_FLOAT){
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
        if(tuple.data[i].type != data_type::NO_DATA){
            return false;
        }
    }
    return true;
}*/


void print(const Tuple& t, long mem_offset){
	for(int i = 0; i < 8; ++i){
		switch(t.data[i].type){
			case data_type::NO_DATA:
				return;
			case data_type::DATA_INT:
				std::cout << t.data[i].data_union.data_int << std::endl;
				break;
			case data_type::DATA_FLOAT:
				std::cout << t.data[i].data_union.data_float << std::endl;
				break;
			case data_type::DATA_STRING:
				std::cout << (t.data[i].data_union.data_string + mem_offset) << std::endl;
				break;
		}
	}
}