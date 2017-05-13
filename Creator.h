#include "tuple.h"

class Creator
{
public:
	Creator(int max_tuples=256, int waiting_size=32);
	~Creator();

private:
	int* memory_addr;
	int fd;
};