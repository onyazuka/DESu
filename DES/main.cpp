// DES.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <ctime>
#include <string>
#include <fstream>
#include "DESFileCrypt.h"



void print_usage()
{
	std::cout << "Usage: DES mode [settings] keys_file input_file output_file\nModes: -e - encrypt, -d - decrypt\n";
	std::cout << "settings: -3 eee3 || ede3 - triple DES\n";
	std::cout << "\t-mt - multithread mode\n";
	std::cout << "Key file generation: DES -g keys_number fname\n";
}


int main(int argc, char* argv[])
{
	srand(time(0));
	File_Crypter crypter;


	if (argc < 4)
	{
		std::cout << "Error! Incorrect arguments count.\n";
		print_usage();
		return(1);
	}

	std::string smode = argv[1];

	// key file generation?
	if (smode == "-g")
	{
		int keys_number = std::stoi(argv[2]);
		if (keys_number != 1 && keys_number != 3)
		{
			std::cout << "Error! Number of keys must be 1 or 3.\n";
			print_usage();
			return(1);
		}
		crypter.ofname = argv[3];
		if (keys_number == 1)
		{
			crypter.set_key(generate_random64());
		}
		else//3
		{
			crypter.set_3keys(generate_random64(), generate_random64(), generate_random64());
		}
		
		if (crypter.write_keys())
		{
			std::cout << "Error! Incorrect file name.\n";
			print_usage();
			return(1);
		}
		std::cout << "Generated.\n";
		return 0;
	}

	if (smode != "-e" && smode != "-d")
	{
		std::cout << "Error! Incorrect mode.\n";
		print_usage();
		return(1);
	}

	int index = 2;
	std::string next_arg;
	next_arg = argv[index++];
	if (next_arg == "-3")	//triple-des
	{
		crypter.triple_des = true;
		next_arg = argv[index++];
		if (next_arg != "eee3" && next_arg != "ede3")
		{
			std::cout << "Error! Triple-DES modes EEE3 and EDE3 only supported.\n";
			print_usage();
			return 1;
		}
		if (next_arg == "eee3")
		{
			crypter.set_triple_des_mode(crypter.EEE3);
		}
		else//EDE3
		{
			crypter.set_triple_des_mode(crypter.EDE3);
		}
	}
	else if (next_arg == "-mt")	//multithread mode
	{
		crypter.multithread = true;
	}
	else
	{
		--index;
	}

	next_arg = argv[index++];
	if (next_arg == "-3")	//triple-des
	{
		crypter.triple_des = true;
		next_arg = argv[index++];
		if (next_arg != "eee3" && next_arg != "ede3")
		{
			std::cout << "Error! Triple-DES modes EEE3 and EDE3 only supported.\n";
			print_usage();
			return 1;
		}
		if (next_arg == "eee3")
		{
			crypter.set_triple_des_mode(crypter.EEE3);
		}
		else//EDE3
		{
			crypter.set_triple_des_mode(crypter.EDE3);
		}
	}
	else if (next_arg == "-mt")	//multithread mode
	{
		crypter.multithread = true;
	}
	else
	{
		--index;
	}
	
	if (argc < index + 3)
	{
		std::cout << "Error! Not enough arguments.\n";
		print_usage();
		return 1;
	}

	crypter.kname = argv[index++];
	crypter.ifname = argv[index++];
	crypter.ofname = argv[index++];

	if (crypter.read_keys())	
	{
		std::cout << "Error! Could not read keys!\n";
		print_usage();
		return 1;
	}
	if (crypter.triple_des && (crypter.keys_size() != 3))			
	{
		std::cout << "Error! Triple-DES was requested, but not enough keys was provided in key file!\n";
		print_usage();
		return 1;
	}

	if (smode == "-d")
	{
		crypter.mode = crypter.Decrypt;
	}
	else
	{
		crypter.mode = crypter.Encrypt;
	}
	clock_t before = clock();
	try
	{
		if (crypter.run())	// error while opening file(s)
		{
			std::cout << "Error! Incorrect file name.\n";
			print_usage();
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
	std::cout << "Elapsed: " << (after - before) << " ms" << std::endl;

	return 0;
}

