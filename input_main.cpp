#include <iostream>
#include "Worker.h"
#include "Creator.h"

void display_help(char* prog_name)
{
	std::cout<<"Usage:"<<std::endl<<std::endl;
	std::cout<<prog_name<<" [-c|-d] <tuple>"<<std::endl;
	std::cout<<"  -c  - creates the shared memory space"<<std::endl;
	std::cout<<"  -d  - deletes the shared memory space and exits"<<std::endl;
}

int parse_arg(char* arg)
{
	std::string argument(arg);
	if(argument == "-c")
	{	
		Creator creator;
		return 0;
	}
	if(argument == "-d")
	{
		Creator creator;
		creator.cleanup();
		exit(0);
	}
	if(arg[0] != 's' && arg[0] != 'i' && arg[0] != 'f')
		return -1;
	return 1;
}

int main(int argc, char** argv)
{
	if(argc == 1 || argc > 3)	
	{	
		display_help(argv[0]);
		return 1;
	}
	int last = 0;
	for(int i = 1; i < argc; ++i)
	{
		if((last = parse_arg(argv[i])) == -1)
		{
			display_help(argv[0]);
			exit(1);
		}
	}
	if(last == 0)
	{
		std::cout<<"No tuple found - it must be the last argument"<<std::endl;
		exit(0);
	}
	std::string pattern(argv[argc-1]);
	Worker worker;
	Tuple t = worker.input(pattern);
	print(t);
}
