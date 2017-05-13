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
		if(addr->data[0].type != NO_DATA)
		{
			for(int i = 0; i < 8; ++i)
			{
				addr->data[i] = tuple.data[i];
				if(tuple.data[i].type == DATA_STRING)
				{
					char * str_addr = (char*)(memory_addr + string_array_offset);
					while(*str_addr != NULL_SIGN)
						str_addr += STRING_SIZE;
					strcpy(str_addr, tuple.data[i].data_union.data_string);
					addr->data[i].data_union.data_string = str_addr;
				}
			}
			return 1;
		}
		else
			addr += TUPLE_SIZE;
	}
	return 0;
}

//return addr of waiting process or nullptr is noone is waiting
Pattern_Pair* Worker::check_waiting_queue(Tuple tuple)
{
	Pattern_Pair* addr = (Pattern_Pair*)(memory_addr + waiting_array_offset);
	while ((int*)addr < memory_addr + shared_size)
	{
		if (*(addr->pattern) != NULL_SIGN)
		{
			if(compare_tuple_with_pattern(tuple, addr->pattern))
				return addr;
		}
		else
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
			sem_wait(addr->sem);
			sem_destroy(addr->sem);
			*(addr->pattern) = NULL_SIGN;
			break;
		}
		else
			addr += sizeof(Pattern_Pair);
	}
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

bool Worker::compare_tuple_with_pattern(Tuple tuple, std::string str)
{
	return true;
}