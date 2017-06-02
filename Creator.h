#ifndef __CREATOR_H_
#define __CREATOR_H_

#include "tuple.h"

class Creator
{
public:
	Creator(int max_tuples=256, int waiting_size=32);
	~Creator();
	void releaseSem();
private:
	char* memory_addr;
	int fd;
};

#endif