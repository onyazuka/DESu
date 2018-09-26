// DES.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <ctime>
#include "DES.h"


int main()
{
	srand(time(0));
	uint64_t block = 15611234987619621009;
	uint64_t key = 67511243098118762;
	int n = 200000;
	clock_t before = clock();
	for (int i = 0; i < n; ++i)
	{
		block = 15611234987619621009;
		key = 67511243098118762;
		//std::cout << "Initial block: " << std::to_string(block) << std::endl;
		uint64_t encr = encrypt(block, key);
		//std::cout << "Encrypted block: " << std::to_string(encr) << std::endl;
		//key = 67511243098118762;
		//uint64_t decr = decrypt(encr, key);
		//std::cout << "Decrypted block: " << std::to_string(decr) << std::endl;
	}
	clock_t after = clock();
	std::cout << "Elapsed: " << (after - before) << std::endl;
	
    return 0;
}

