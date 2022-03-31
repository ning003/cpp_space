#include <iostream>
#include "nfork.h"

int main(){
	std::cout<<"Runn Main ===="<<std::endl;
	Nfork *n = new Nfork();
	n->RunFork();

	return 0;
}
