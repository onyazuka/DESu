// DES.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DES.h"




int main()
{
	uint64_t block = 15611234987619621009;
	uint64_t key = 67511243098118762;
	std::cout << "Initial block: " << std::to_string(block) << std::endl;
	encrypt(&block, &key);
	std::cout << "Encrypted block: " << std::to_string(block) << std::endl;
	decrypt(&block, &key);
	std::cout << "Decrypted block: " << std::to_string(block) << std::endl;
	//permutation(uch1, 64, initial_permutation_array);
    return 0;
}

