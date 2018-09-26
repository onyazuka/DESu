// DES.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

#include"DES.h"

/*
Doing 4bits block B' from 6bits block B by transformation tables;
Example: if B is 101111, than we are taking bits 1 and 6(11) and bits 2-5(0111) and looking
row 3(11) and column 7(0111) of transformation table number table_num.
*/
void DESEncrypter::narrow_block(uchar& B, int table_num)
{
	int row = ((B & (1 << 5)) >> 4) + (B & 1);
	int col = (B & 0b011110) >> 1;
	B = (*trans_table[table_num])[row][col];
}

/*
Append 56 bits key to 64 key so number of 1 bits in each byte becomes odd.
Using for checking correctness.
*/
void DESEncrypter::append_key_to_odd()
{
	uint64_t new_key = 0;
	uint64_t cur_key = key;
	for (int i = 0; i < 8; ++i)
	{
		std::bitset<8> byte((cur_key & ((uint64_t)0b01111111 << (i * 7))) >> (i * 7));
		// even count of 1 bits - adding 1 as msb
		if (byte.count() % 2 == 0)
		{
			byte[7] = 1;
		}
		new_key += (byte.to_ullong() << (8 * i));
	}
	key = new_key;
}

/*
takes 7 least significant bits from every byte of provided 64 bits
provides uint64_t number from 56 resulting bits
*/
void DESEncrypter::take_7bits()
{
	uint64_t val = key;
	uint64_t new_val = 0;
	for (int i = 0; i < 8; ++i)
	{
		new_val += (val & ((uint64_t)0b01111111 << (i * 8))) >> i;
	}
	key = new_val;
}

// does from 48bits passed value 8 blocks of 6 bits
// (0 is eldest)
void DESEncrypter::fill_6bits_blocks(uint64_t& number, std::array<uchar, 8>& blocks)
{
	blocks[0] = (static_cast<ushort>(number) & 0b111111);
	blocks[1] = ((static_cast<ushort>(number) & 0b111111000000) >> 6);
	blocks[2] = ((static_cast<ushort>(*reinterpret_cast<uint64_t*>((reinterpret_cast<uchar*>(&number) + 1))) & 0b1111110000) >> 4);
	blocks[3] = ((static_cast<ushort>(*reinterpret_cast<uint64_t*>((reinterpret_cast<uchar*>(&number) + 2))) & 0b11111100) >> 2);
	blocks[4] = ((static_cast<ushort>(*reinterpret_cast<uint64_t*>((reinterpret_cast<uchar*>(&number) + 2))) & 0b11111100000000) >> 8);
	blocks[5] = ((static_cast<ushort>(*reinterpret_cast<uint64_t*>((reinterpret_cast<uchar*>(&number) + 3))) & 0b111111000000) >> 6);
	blocks[6] = ((static_cast<ushort>(*reinterpret_cast<uint64_t*>((reinterpret_cast<uchar*>(&number) + 4))) & 0b1111110000) >> 4);
	blocks[7] = ((static_cast<ushort>(*reinterpret_cast<uint64_t*>((reinterpret_cast<uchar*>(&number) + 5))) & 0b11111100) >> 2);
}

// Feistel function
uint32_t DESEncrypter::feistel(uint32_t& ri_minus1, int round)
{
	uint64_t ri_copy = ri_minus1;
	transformation(ri_copy, EXPANDED_HALF_BLOCK_SIZE, expanding_array); // now 48 bits
																		//ri_minus1 = ri_copy;
	ri_copy = modulo2_addition(ri_copy, key);
	uint64_t res = 0;
	std::array<uchar, 8> blocks;
	fill_6bits_blocks(ri_copy, blocks);
	for (int i = 0; i < 8; ++i)
	{
		//taking by 6 bits
		uchar temp = blocks[i];
		narrow_block(temp, i);
		res += ((uint64_t)temp << (i * 4));
	}
	transformation(res, HALF_BLOCK_SIZE, f_final_permutation_array);
	return (uint32_t)(res);
}

void DESEncrypter::init_C0_and_D0(uint32_t& C0, uint32_t& D0)
{
	append_key_to_odd();
	//???where we are using this append???(used for correctness checking, but not here)
	transformation(key, BLOCK_SIZE, key_permutation_array);
	take_7bits();
	// because we have 28 significant bits, and uint32_t has 32
	C0 = (key & ((uint64_t)0xfffffff << 28)) >> 28;
	D0 = key & 0xfffffff;
}

void DESEncrypter::update_key(uint32_t& C, uint32_t& D, int round)
{
	if (mode == Mode::ENCRYPT)
	{
		C = rol(C, key_round_encrypt_shifting_array[round], 28);
		D = rol(D, key_round_encrypt_shifting_array[round], 28);
	}
	else    //decrypt
	{
		C = ror(C, key_round_decrypt_shifting_array[round], 28);
		D = ror(D, key_round_decrypt_shifting_array[round], 28);
	}
	uint64_t CD = ((uint64_t)C << 28) + D;
	transformation(CD, FINAL_KEY_SIZE, key_selection_array);
	key = CD;
}

/*
encrypts block of data(64 bits)
Li - 32 most significant bits,
Ri - 32 least significant bits,
Ki - key
*/
uint64_t DESEncrypter::run()
{
	transformation(block, BLOCK_SIZE, initial_permutation_array);
	uint32_t Li = ((block & ((uint64_t)0xffffffff << 32)) >> 32);
	uint32_t Ri = block & 0xffffffff;
	uint64_t ki = key;
	uint32_t Ci = 0;
	uint32_t Di = 0;
	init_C0_and_D0(Ci, Di);
	const int ROUNDS = 16;
	for (int i = 0; i < ROUNDS; ++i)	//16 rounds of encrypting
	{
		update_key(Ci, Di, i);
		uint32_t ltemp = Li;
		if (mode == Mode::ENCRYPT)
		{
			Li = Ri;
			Ri = modulo2_addition(ltemp, feistel(Ri, i));
		}
		else    //decrypt
		{
			Li = modulo2_addition(Ri, feistel(Li, i));
			Ri = ltemp;
		}
	}
	block = ((uint64_t)Li << 32) + Ri;
	transformation(block, BLOCK_SIZE, final_permutation_array);
	return block;
}

/*
Addition modulo 2 of two 64 bits numbers(blocks), which in fact may be 48 bits expanded half-blocks
*/
uint64_t modulo2_addition(uint64_t num1, uint64_t num2)
{
	return num1 ^ num2;
}
