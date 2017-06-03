#include <iostream>
#include <unistd.h>
#include "Creator.h"

int main(){
	Creator creator;
	char x;
	while(true){
		std::cout << "Press x to exit: ";
		std::cin >> x;
		if(x == 's')
			creator.releaseSem(); 
		if(x == 'x')
			break;
	}
}