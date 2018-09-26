// DES.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <ctime>
#include <string>
#include <fstream>
#include "DES.h"
#include "DESTechTools.h"

const int BUFSIZE = 1024 * 1024;
const int BLOCKSIZE = 8;

int process_file(const std::string& ifname, std::string ofname, uint64_t key, int mode)
{
	// opening files
	std::ifstream ifs;
	ifs.open(ifname, std::ios_base::binary);
	std::ofstream ofs;
	ofs.open(ofname, std::ios_base::binary);
	if (!ifs || !ofs)
	{
		return -1;
	}
	char* buffer = new char[BUFSIZE];
	ifs.read(buffer, BUFSIZE);
	uint64_t res;
	while (ifs.gcount() != 0)
	{
		// alignning data to 64 bits
		int to_align = (BLOCKSIZE - ifs.gcount() % BLOCKSIZE) == BLOCKSIZE ? 0 : (BLOCKSIZE - ifs.gcount() % BLOCKSIZE);
		int to_read = ifs.gcount() + to_align;
		memset(buffer + ifs.gcount(), 0, to_align);
		// processing blocks
		for (int i = 0; i < to_read; i += BLOCKSIZE)
		{
			DESEncrypter encrypter{ *reinterpret_cast<uint64_t*>(buffer + i), key, mode };
			res = encrypter.run();
			//changing buffer - we don't need processed data anyways
			memcpy(buffer + i, &res, BLOCKSIZE);
		}
		ofs.write(buffer, ifs.gcount());
		ifs.read(buffer, BUFSIZE);
	}
	delete[] buffer;
	return 0;
}


int main(int argc, char* argv[])
{

	/*srand(time(0));
	if (argc != 4)
	{
		std::cout << "Error! Incorrect arguments count.\n";
		std::cout << "Usage: DES mode input_file output_file\nModes: -e - encrypt, -d - decrypt\n";
		return(1);
	}

	struct input_data
	{
		std::string mode;
		std::string ifname;
		std::string ofname;
		std::string password;
	} idata;

	idata.mode = argv[1];
	idata.ifname = argv[2];
	idata.ofname = argv[3];

	if (idata.mode != "-e" && idata.mode != "-d")
	{
		std::cout << "Error! Incorrect mode.\n";
		std::cout << "Usage: DES mode input_file output_file\nModes: -e - encrypt, -d - decrypt\n";
		return(1);
	}

	std::cout << "Enter password:\n";
	std::cin >> idata.password;
	int err = 0;
	uint64_t key = password_from_string_to_uint64(idata.password, err);
	if (err == 1)
	{
		std::cout << "Warning! Key is too small(using 7 bytes keys).\n";
	}
	else if (err == 2)
	{
		std::cout << "Warning! Key is too big(using 7 bytes keys). It was cutted.\n";
	}

	int mode = 0;
	if (idata.mode == "-d")
	{
		mode = 0;
	}
	else
	{
		mode = 1;
	}
	clock_t before = clock();
	try
	{
		if (process_file(idata.ifname, idata.ofname, key, mode))	// error while opening file(s)
		{
			std::cout << "Error! Incorrect file name.\n";
			std::cout << "Usage: DES mode input_file output_file\nModes: -e - encrypt, -d - decrypt\n";
			return(1);
		}
		else
		{
			std::cout << "Done\n";
		}
	}
	catch (std::runtime_error& err)
	{
		std::cout << "Error while processing: " << err.what() << ".\n";
		return(1);
	}
	clock_t after = clock();
	std::cout << "Elapsed: " << (after - before) << " ms" << std::endl;*/
	

	uint64_t block = 15611234987619621009;
	uint64_t key = 67511243098118762;
	int n = 200000;
	clock_t before = clock();
	for (int i = 0; i < n; ++i)
	{
		block = 15611234987619621009;
		key = 67511243098118762;
		//std::cout << "Initial block: " << std::to_string(block) << std::endl;
		DESEncrypter encrypter(block, key, 0);
		uint64_t encr = encrypter.run();
		//std::cout << "Encrypted block: " << std::to_string(encr) << std::endl;
		//key = 67511243098118762;
		//DESEncrypter decrypter(encr, key, 1);
		//uint64_t decr = decrypter.run();
		//std::cout << "Decrypted block: " << std::to_string(decr) << std::endl;
	}
	clock_t after = clock();
	std::cout << "Elapsed: " << (after - before) << std::endl;

	return 0;
}

