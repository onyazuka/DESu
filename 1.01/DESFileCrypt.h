#pragma once
#include <fstream>
#include "DES.h"
#include "DESTechTools.h"

const int BUFSIZE = 1024 * 1024;
const int BLOCKSIZE = 8;

class File_Crypter
{
public:
	static enum Modes { Decrypt, Encrypt, Gen_Keys };
	static enum Triple_DES_Modes {EEE3, EDE3};
	int mode;
	std::string ifname;
	std::string ofname;
	std::string kname;
	bool triple_des = false;
	bool multithread = false;

	int run();
	int write_keys();
	int read_keys();
	void set_key(uint64_t key);
	void set_3keys(uint64_t key1, uint64_t key2, uint64_t key3);
	int get_key(int index, uint64_t& key) const;
	inline int keys_size() const { return keys_number; }
	int set_triple_des_mode(int mode);
	inline int get_triple_des_mode() const { return triple_des_mode; }
private:
	int keys_number = 0;
	std::array<uint64_t, 3> keys;
	int triple_des_mode;
	uint64_t encrypt_tiple_des(char* buffer, int offset);
	uint64_t decrypt_tiple_des(char* buffer, int offset);
	uint64_t run_des(char* buffer, int offset);
};