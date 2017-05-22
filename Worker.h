#include "tuple.h"
#include <string.h>
#include <string>
#include <vector>
#include <sstream>
#if DEBUG
#include <iostream>
#endif
class Worker
{
public: 
	Worker();
	~Worker();
	void output(Tuple tuple);
	Tuple input(std::string pattern);
	Tuple read(std::string pattern);
private:
	int* memory_addr;
	int tuple_array_offset;
	int string_array_offset;
	int waiting_array_offset;
	int shared_size;

	int fd;

	void read_meta_data(int fd);
	void lock_semaphore();
	void unlock_semaphore();
	int add_tuple_to_memory(Tuple tuple);
	int add_string_to_memory(char * memory_tuple_str, char * user_tuple_str);
	Pattern_Pair* check_waiting_queue(Tuple tuple);
	Tuple* find_tuple_in_memory(std::string pattern);
	void wake_up_process(Pattern_Pair* waiting_addr);
	int wait_in_memory_for_tuple(std::string pattern);
	Tuple read_tuple_from_memory(Tuple* addr);
	Tuple remove_tuple_from_memory(Tuple* addr);

	bool compare_tuple_with_pattern(Tuple tuple, std::string pattern);
};
