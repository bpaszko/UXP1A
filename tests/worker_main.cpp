#include <iostream>
#include <unistd.h>
#include "Worker.h"


void input(Worker& worker){
	std::string pattern;
	std::cout << "Pattern: ";
	std::cin >> pattern;
	Tuple t = worker.input(pattern);
	print(t);
}

void read(Worker& worker){
	std::string pattern;
	std::cout << "Pattern: ";
	std::cin >> pattern;
	Tuple t = worker.read(pattern);
	print(t);
}

void output(Worker& worker){
	char type = 'n';
	int myint;
	float myfloat;
	char mystring[64];
	Tuple tuple;
	for(int i = 0; i < 8 && type != 'x'; ++i){
		std::cout << "Type: ";
		std::cin >> type;
		switch(type){
			case 'f':
				tuple.data[i].type = data_type::DATA_FLOAT;
				std::cout << "Float: ";
				std::cin >> myfloat;
				tuple.data[i].data_union.data_float = myfloat;
				break;
			case 'i':
				tuple.data[i].type = data_type::DATA_INT;
				std::cout << "Int: ";
				std::cin >> myint;
				tuple.data[i].data_union.data_int = myint;
				break;
			case 's':
				tuple.data[i].type = data_type::DATA_STRING;
				std::cout << "String: ";
				std::cin >> mystring;
				tuple.data[i].data_union.data_string = mystring;
				break;
		}
	}
	worker.output(tuple);
}

void fun(Worker& worker){
	//Tuple t1;
	//Tuple t2;
	Tuple t3;
	//t1.data[0].type = data_type::DATA_FLOAT;
	//t1.data[0].data_union.data_float = 12.2;
	//t1.data[1].type = data_type::DATA_INT;
	//t1.data[1].data_union.data_int = 10;
	//t2.data[0].type = data_type::DATA_INT;
	//t2.data[0].data_union.data_int = 2;
	//t2.data[1].type = data_type::DATA_INT;
	//t2.data[1].data_union.data_int = 3;

	t3.data[0].type = data_type::DATA_INT;
	t3.data[0].data_union.data_int = 2;
	t3.data[1].type = data_type::DATA_STRING;
	char * s1 = "hello";
	char * s2 = "world";
	t3.data[1].data_union.data_string = s1;
	t3.data[2].type = data_type::DATA_STRING;
	t3.data[2].data_union.data_string = s2;
	//worker.output(t1);
	worker.output(t3);
	//worker.output(t2);
	//Tuple t4 = worker.input("i:2,s:\"hello\",s:\"Puszka\"");
	//print(t4);
}

int main(){
	Worker worker;

	char x;
	while(true){
		std::cout << "Action: ";
		std::cin >> x;
		switch(x){
			case 'k':
				fun(worker);
				break;
			case 'o':
				output(worker);
				break;
			case 'i':
				input(worker);
				break;
			case 'r':
				read(worker);
				break;
			case 'x':
				return 0;
			default:
				break;
		}
	}

}