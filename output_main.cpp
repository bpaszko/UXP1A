#include <iostream>
#include "Worker.h"
#include "Creator.h"
#include <exception>

void fill_data_in_tuple(Tuple& tuple, int i, std::string& current_token)
{
	switch(char(current_token[0])){
        		case 'f':
        			tuple.data[i].type = data_type::DATA_FLOAT;
        			tuple.data[i].data_union.data_float = stof(current_token.substr(2));
        			break;
        		case 'i':
        			tuple.data[i].type = data_type::DATA_INT;
        			tuple.data[i].data_union.data_int = stoi(current_token.substr(2));
        			break;
        		case 's':
        			tuple.data[i].type = data_type::DATA_STRING;
        			int len = strlen((current_token.substr(2)).c_str())+1;
        			if(len > 64)
        				throw std::exception();
        			tuple.data[i].data_union.data_string = new char[len];
        			strcpy(tuple.data[i].data_union.data_string, (current_token.substr(2)).c_str());
        			break;
        	}
}


Tuple& pattern_to_tuple(std::string pattern, Tuple& tuple)
{
    bool flag = false;
    int tokens = 0;
    std::string current_token;
    int i = 0;
    for(i=0; i<pattern.size() && tokens < 8; ++i)
    {
        if(pattern[i]=='\"')
        {
            flag = !flag;
            continue;
        }
        if(pattern[i]==',' && !flag){
        	fill_data_in_tuple(tuple, tokens, current_token);
        	print(tuple);
        	current_token = "";
            tokens++;
        }
        else
            current_token += pattern[i];
    }
    if(current_token != "")
    	fill_data_in_tuple(tuple, tokens, current_token);
    return tuple;
}

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
	Tuple tuple;
	pattern_to_tuple(pattern, tuple);
	worker.output(tuple);
}
