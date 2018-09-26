#include "stdafx.h"
#include "DESFileCrypt.h"


/*
Used both in signlethread and multithread versions.
Processes 'blocks' blocks in one function. Blocks size assumed to be sizeof(uint64_t)
*/
void File_Crypter::run_des(char* buffer, int blocks)
{
	
	thread_local uint64_t res = 0;
	if (!triple_des)	//DES
	{
		for (int block = 0; block < blocks; ++block)
		{
			DESEncrypter encrypter{ *reinterpret_cast<uint64_t*>(buffer + sizeof(uint64_t) * block), keys[0], mode };
			res = encrypter.run();
			//changing buffer - we don't need processed data anyways
			memcpy(buffer + sizeof(uint64_t) * block, &res, BLOCKSIZE);
		}
	}
	else                //Triple-DES
	{
		if (mode == Modes::Decrypt)
		{
			for (int block = 0; block < blocks; ++block)
			{
				res = decrypt_tiple_des(buffer + sizeof(uint64_t) * block);
				memcpy(buffer + sizeof(uint64_t) * block, &res, BLOCKSIZE);
			}
		}
		else if (mode == Modes::Encrypt)
		{
			for (int block = 0; block < blocks; ++block)
			{
				res = encrypt_tiple_des(buffer + sizeof(uint64_t) * block);
				memcpy(buffer + sizeof(uint64_t) * block, &res, BLOCKSIZE);
			}
		}
	}
}

uint64_t File_Crypter::encrypt_tiple_des(char* buffer)
{
	uint64_t res = *reinterpret_cast<uint64_t*>(buffer);
	if (triple_des_mode == Triple_DES_Modes::EEE3)
	{
		for (int key = 0; key < 3; ++key)
		{
			DESEncrypter encrypter{ res, keys[key], mode };
			res = encrypter.run();
		}
	}
	else if (triple_des_mode == Triple_DES_Modes::EDE3)
	{
		for (int key = 0; key < 3; ++key)
		{
			// key % 2 - 010 - encrypt - decrypt - encrypt
			DESEncrypter encrypter{ res, keys[key], key % 2 };
			res = encrypter.run();
		}
	}
	return res;
}

uint64_t File_Crypter::decrypt_tiple_des(char* buffer)
{
	uint64_t res = *reinterpret_cast<uint64_t*>(buffer);
	if (triple_des_mode == Triple_DES_Modes::EEE3)
	{
		for (int key = 0; key < 3; ++key)
		{
			DESEncrypter encrypter{ res, keys[2 - key], mode };
			res = encrypter.run();
		}
	}
	else if (triple_des_mode == Triple_DES_Modes::EDE3)
	{
		for (int key = 0; key < 3; ++key)
		{
			// key % 2 - 010 - encrypt - decrypt - encrypt
			DESEncrypter encrypter{ res, keys[2 - key], 1 - (key % 2) };
			res = encrypter.run();
		}
	}
	return res;
}

int File_Crypter::run()
{
	if (multithread)
	{
		return run_mt();
	}
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
	int done = 0;
	while (ifs.gcount() != 0)
	{
		// alignning data to 64 bits
		int to_align = (BLOCKSIZE - ifs.gcount() % BLOCKSIZE) == BLOCKSIZE ? 0 : (BLOCKSIZE - ifs.gcount() % BLOCKSIZE);
		int to_read = ifs.gcount() + to_align;
		memset(buffer + ifs.gcount(), 0, to_align);
		// processing blocks
		for (int i = 0; i < to_read; i += BLOCKSIZE)
		{
			run_des(buffer + i, 1);
		}
		ofs.write(buffer, to_read);
		ifs.read(buffer, BUFSIZE);
	}
	delete[] buffer;
	return 0;
}


/*
writing generated keys into keyfile
*/
int File_Crypter::write_keys()
{
	std::ofstream ofs;
	ofs.open(ofname, std::ios_base::binary);
	if (!ofs)
	{
		return -1;
	}
	for (int i = 0; i < keys_number; ++i)
	{
		ofs.write(reinterpret_cast<const char*>(&keys[i]), sizeof(uint64_t));
	}
	return 0;
}

//read keys from keyfile
int File_Crypter::read_keys()
{
	std::ifstream ifs;
	ifs.open(kname, std::ios_base::binary);
	if (!ifs)
	{
		return -1;
	}
	int n = 0;
	while (ifs && (n != 3))	//3 keys max
	{
		ifs.read(reinterpret_cast<char*>(&keys[n]), sizeof(uint64_t));
		n++;
	}
	keys_number = n;
	return 0;
}

// init key storage for simple des
void File_Crypter::set_key(uint64_t key)
{
	keys[0] = key;
	keys_number = 1;
}

// for triple-DES
void File_Crypter::set_3keys(uint64_t key1, uint64_t key2, uint64_t key3)
{
	keys[0] = key1;
	keys[1] = key2;
	keys[2] = key3;
	keys_number = 3;
}

// Gets key number index from key storage. Maximum number of keys - 3(for triple DES)
// returns -1 if key was not gotten, else returns 0
int File_Crypter::get_key(int index, uint64_t& key) const
{
	if (index < 0 || index > keys_number)
	{
		return -1;
	}
	key = keys[index];
	return 0;
}

/*
returns -1 on incorrect mode,
else returns 0
*/
int File_Crypter::set_triple_des_mode(int tdmode)
{
	if (tdmode != Triple_DES_Modes::EDE3 && tdmode != Triple_DES_Modes::EEE3)
	{
		return -1;
	}
	triple_des_mode = tdmode;
	return 0;
}

/*----------------------------MULTITHREAD----------------------------*/

/*
Multithread version of run
*/
int File_Crypter::run_mt()
{
	// opening files
	int done = 0;
	std::ifstream ifs;
	ifs.open(ifname, std::ios_base::binary);
	std::ofstream ofs;
	ofs.open(ofname, std::ios_base::binary);
	if (!ifs || !ofs)
	{
		return -1;
	}
	ThreadPoolMy thread_pool;
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
		int offset = 0;
		for (int i = 0; i < thread_pool.size() - 1; ++i)
		{
			thread_pool.wait_do_task(std::bind(&File_Crypter::run_des, this, buffer + offset, to_read / BLOCKSIZE / thread_pool.size()));
			offset += to_read / thread_pool.size();
		}
		// processing last portion of blocks
		thread_pool.wait_do_task(std::bind(&File_Crypter::run_des, this, buffer + offset, (to_read - offset) / BLOCKSIZE));
		thread_pool.wait_all_tasks();
		ofs.write(buffer, to_read);
		ifs.read(buffer, BUFSIZE);
	}
	// waiting before deleting buffer
	thread_pool.wait_all_tasks();
	delete[] buffer;
	return 0;
}