#include "Creator.h"
#include <iostream>
#include <cstring>

Creator::Creator(int max_tuples, int waiting_size)
{
	Meta_Data meta;
	meta.tuple_array_offset = META_SIZE;
	meta.string_array_offset = META_SIZE + max_tuples*TUPLE_SIZE;
	meta.waiting_array_offset = meta.string_array_offset + max_tuples*8*STRING_SIZE;
	meta.shared_size = meta.waiting_array_offset + sizeof(Pattern_Pair)*waiting_size;

	//sem_init(meta.main_sem,1,0);
	fd = shm_open("/tuple_space", O_CREAT|O_RDWR, 0777);
	ftruncate(fd, meta.shared_size);
	void* mem_addr = mmap(0, meta.shared_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	memory_addr = (char*)mem_addr;
	Meta_Data* addr = (Meta_Data*)memory_addr;
	std::cout << addr << std::endl;
	*addr = meta;
	if (!sem_init(&(addr->main_sem), 1, 1))
		std::cout << "SUCCESS" << std::endl;
	
	//INIT TUPLE_ARRAY
	Tuple* tuple_addr = (Tuple*)(memory_addr + meta.tuple_array_offset);
	Tuple tuple;
	for(int i = 0; i < max_tuples; ++i)
	{
		*tuple_addr = tuple;
		tuple_addr = (Tuple*)((char*)tuple_addr + TUPLE_SIZE);
	}


	//INIT STRINGS
	char* str = (char*)(memory_addr + meta.string_array_offset);
	for(int i = 0; i < max_tuples*8; ++i)
	{
		*str = NULL_SIGN;
		str = (char*)(str + STRING_SIZE);
	}
	
	//INIT WAITING ARRAY
	Pattern_Pair pair;
	*pair.pattern = NULL_SIGN;
	Pattern_Pair* wait_addr = (Pattern_Pair*)(memory_addr + meta.waiting_array_offset);
	for(int i = 0; i < waiting_size; ++i)
	{
		*wait_addr = pair;
		wait_addr = (Pattern_Pair*)((char*)wait_addr + sizeof(Pattern_Pair));
	}
	std::cout << meta.shared_size  << " " << (char*)wait_addr-memory_addr << std::endl;
}

void Creator::releaseSem()
{
	Meta_Data* meta = (Meta_Data*)memory_addr;
	sem_post(&(meta->main_sem));	
}

void Creator::cleanup()
{
	Meta_Data* meta = (Meta_Data*)memory_addr;
	sem_destroy(&(meta->main_sem));
	//delete meta->main_sem;
	munmap((void*)memory_addr, meta->shared_size);
	close(fd);
	shm_unlink("tuple_space");
}

Creator::~Creator()
{

}
