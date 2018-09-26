#pragma once
#include <array>
#include <bitset>
#include <string>
#include <iostream>
#include <memory>
#include <stdint.h>

static const int BLOCK_SIZE = 64;
static const int HALF_BLOCK_SIZE = 32;
static const int EXPANDED_HALF_BLOCK_SIZE = 48;
static const int KEY_SIZE = 56;
static const int EXPANDED_KEY_SIZE = 64;
static const int FINAL_KEY_SIZE = 48;

typedef unsigned char uchar;
typedef unsigned long long ull;
typedef std::array<std::array<int, 16>, 4> sub_table;
typedef std::array<const sub_table*, 8> transformation_table;

// bit 7 of initial block becomes bit 1 of output block(LSB)
static const std::array<int, BLOCK_SIZE> initial_permutation_array
{ 58, 50, 42, 34, 26, 18, 10, 2,
60, 52, 44, 36, 28, 20, 12, 4,
62, 54, 46, 38, 30, 22, 14, 6,
64, 56, 48, 40, 32, 24, 16, 8,
57, 49, 41, 33, 25, 17, 9,  1,
59, 51, 43, 35, 27, 19, 11, 3,
61, 53, 45, 37, 29, 21, 13, 5,
63, 55, 47, 39, 31, 23, 15, 7 };

static const std::array<int, EXPANDED_HALF_BLOCK_SIZE> expanding_array
{
	32,1, 2, 3, 4, 5,
	4, 5, 6, 7, 8, 9,
	8, 9, 10,11,12,13,
	12,13,14,15,16,17,
	16,17,18,19,20,21,
	20,21,22,23,24,25,
	24,25,26,27,28,29,
	28,29,30,31,32,1
};

static const sub_table st0
{
	{
		{ 14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7 },
		{ 0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8 },
		{ 4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0 },
		{ 15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13 }
	}
};
static const sub_table st1
{
	{
		{ 15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10 },
		{ 3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5 },
		{ 0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15 },
		{ 13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9 }
	}
};
static const sub_table st2
{
	{
		{ 10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8 },
		{ 13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1 },
		{ 13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7 },
		{ 1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12 }
	}
};
static const sub_table st3
{
	{
		{ 7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15 },
		{ 13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9 },
		{ 10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4 },
		{ 3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14 }
	}
};
static const sub_table st4
{
	{
		{ 2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9 },
		{ 14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6 },
		{ 4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14 },
		{ 11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3 }
	}
};
static const sub_table st5
{
	{
		{ 12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11 },
		{ 10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8 },
		{ 9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6 },
		{ 4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13 }
	}
};
static const sub_table st6
{
	{
		{ 4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1 },
		{ 13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6 },
		{ 1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2 },
		{ 6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12 }
	}
};
static const sub_table st7
{
	{
		{ 13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7 },
		{ 1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2 },
		{ 7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8 },
		{ 2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11 }
	}
};

static const transformation_table trans_table
{
	&st0, &st1, &st2, &st3, &st4, &st5, &st6, &st7
};

static const std::array<int, HALF_BLOCK_SIZE> f_final_permutation_array
{
	16,7, 20,21,29,12,28,17,
	1, 15,23,26,5, 18,31,10,
	2, 8, 24,14,32,27,3, 9,
	19,13,30,6, 22,11,4, 25
};

static const std::array<int, 64> key_permutation_array
{
	64,57,49,41,33,25,17, 9,56,1, 58,50,42,34,26,18,
	48,10,2, 59,51,43,35,27,40,19,11,3, 60,52,44,36,
	32,63,55,47,39,31,23,15,24,7, 62,54,46,38,30,22,
	16,14,6, 61,53,45,37,29,8, 21,13,5, 28,20,12,4
};

/*static const std::array<int, EXPANDED_KEY_SIZE> key_permutation_array
{
57,49,41,33,25,17,9, 1, 58,50,42,34,26,18,
10,2, 59,51,43,35,27,19,11,3, 60,52,44,36,
63,55,47,39,31,23,15,7, 62,54,46,38,30,22,
14,6, 61,53,45,37,29,21,13,5, 28,20,12,4
};*/

static const std::array<int, 16> key_round_encrypt_shifting_array
{
	1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1
};

static const std::array<int, 16> key_round_decrypt_shifting_array
{
	0,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1
};



static const std::array<int, FINAL_KEY_SIZE> key_selection_array
{
	14,17,11,24,1, 5, 3, 28,15,6, 21,10,23,19,12,4,
	26,8, 16,7, 27,20,13,2, 41,52,31,37,47,55,30,40,
	51,45,33,48,44,49,39,56,34,53,46,42,50,36,29,32
};

static const std::array<int, BLOCK_SIZE> final_permutation_array
{
	40,8, 48,16,56,24,64,32,39,7, 47,15,55,23,63,31,
	38,6, 46,14,54,22,62,30,37,5, 45,13,53,21,61,29,
	36,4, 44,12,52,20,60,28,35,3, 43,11,51,19,59,27,
	34,2, 42,10,50,18,58,26,33,1, 41,9, 49,17,57,25
};

/*
	Rotating left
	digits - number of significant digits(so we can use uint64_t with 56 significant digits)
	Warning: rotating, for example, uchar to 10 will equal to 0!!!
*/
template<typename UINT>
constexpr UINT rol(UINT num, int val, int digits = -1)
{
	static_assert(std::is_unsigned<UINT>::value, "Rotate left only makes sense with unsigned types!");
	if (digits < -1)
	{
		throw(std::runtime_error("rotate left: digits can not be < -1"));
	}
	UINT mask = ((UINT)1 << digits) - 1;
	if (digits == -1)
	{
		digits = sizeof(UINT) * CHAR_BIT;
		mask = ((UINT)1 << digits) - 1;
	}
	num &= mask;
	return ((num << val) | (num >> (digits - val))) & mask;
}

/*
	Rotating right
	digits - number of significant digits(so we can use uint64_t with 56 significant digits)
	Warning: rotating, for example, uchar to 10 will equal to 0!!!
*/
template<typename UINT>
constexpr UINT ror(UINT num, int val, int digits = -1)
{
	static_assert(std::is_unsigned<UINT>::value, "Rotate right only makes sense with unsigned types!");
	if (digits < -1)
	{
		throw(std::runtime_error("rotate left: digits can not be < -1"));
	}
	UINT mask = ((UINT)1 << digits) - 1;
	if (digits == -1)
	{
		digits = sizeof(UINT) * CHAR_BIT;
		mask = ((UINT)1 << digits) - 1;
	}
	num &= mask;
	return (num >> val) | (num << (digits - val)) & mask;
}


/*
Doing permutation on data block which is size bits size.
Using perm_array for permutations(which must also has len=size)
NOTE: bit 1 is always most significant bit
*/
template<typename T>
void permutation(uint64_t& block, int size, T& perm_array)
{
	if (perm_array.size() != size)
	{
		throw(std::runtime_error{ "permutation: perm_array must be " + std::to_string(size) + " size\n" });
	}
	// 64bits = 8bytes, can be represented as ull
	uint64_t block_copy = block;
	uint64_t res = 0;
	bool temp = true;
	for (int i = 0; i < size; ++i)
	{
		// only need flag if bit set or not
		temp = (block_copy & ((uint64_t)1 << (perm_array[size - 1 - i]) - 1));
		//block ^= block & ((uint64_t)1 << i);
		if (temp)
		{
			res |= ((uint64_t)1 << i);
		}
		
	}
	block = res;
}

/*
	Function expanding or narrowing block of data.
	Products new block, and we want to pass owning of it, so we are using std::unique_ptr
*/
template<typename T>
uint64_t transformation(uint64_t& block, int size, T& select_array)
{

	// 64bits = 8bytes, can be represented as ull
	uint64_t block_copy = block;
	uint64_t res = 0;
	bool temp = true;
	for (int i = 0; i < size; ++i)
	{
		temp = (block_copy & ((uint64_t)1 << (select_array[size - 1 - i]) - 1));
		if (temp)
		{
			res |= ((uint64_t)1 << i);
		}
	}
	return res;
}

/*
	Addition modulo 2 of two 64 bits numbers(blocks), which in fact may be 48 bits expanded half-blocks
*/
uint64_t modulo2_addition(uint64_t num1, uint64_t num2)
{
	return num1 ^ num2;
}

/*
	Doing 4bits block B' from 6bits block B by transformation tables;
	Example: if B is 101111, than we are taking bits 1 and 6(11) and bits 2-5(0111) and looking 
	row 3(11) and column 7(0111) of transformation table number table_num.
*/
void narrow_block(uchar& B, int table_num)
{
	int row = ((B & (1 << 5)) >> 4) + (B & 1);
	int col = (B & 0b011110) >> 1;
	B = (*trans_table[table_num])[row][col];
}

/*
	Append 56 bits key to 64 key so number of 1 bits in each byte becomes odd.
	Using for checking correctness.
*/
void append_key_to_odd(uint64_t& key)
{
	uint64_t new_key = 0;
	uint64_t cur_key = key;
	for (int i = 0; i < 8; ++i)
	{
		std::bitset<8> byte((cur_key & ((uint64_t)0b01111111 << ((uint64_t)i * 7))) >> ((uint64_t)i * 7));
		// even count of 1 bits - adding 1 as msb
		if (byte.count() % 2 == 0)
		{
			byte[7] = 1;
		}
		new_key += (byte.to_ullong() << ((uint64_t)8 * i));
	}
	key = new_key;
}

/*
	takes 7 least significant bits from every byte of provided 64 bits
	provides uint64_t number from 56 resulting bits
*/
void take_7bits(uint64_t& key)
{
	uint64_t val = key;
	uint64_t new_val = 0;
	for (int i = 0; i < 8; ++i)
	{
		new_val += (val & ((uint64_t)0b01111111 << ((uint64_t)i * 8))) >> i;
	}
	key = new_val;
}


// Функция Фейстеля
uint32_t feistel(uint32_t& ri_minus1, uint64_t& ki, int round, bool encrypt)
{
	uint64_t ri_copy = ri_minus1;
	uint64_t expanded_r = transformation(ri_copy, EXPANDED_HALF_BLOCK_SIZE, expanding_array); // now 48 bits
	ri_minus1 = ri_copy;
	expanded_r = modulo2_addition(expanded_r, ki);
	uint64_t res = 0;
	for (int i = 0; i < 8; ++i)
	{
		//taking by 6 bits
		uchar temp = (expanded_r & ((uint64_t)0b111111 << ((uint64_t)i * 6))) >> ((uint64_t)i * 6);
		narrow_block(temp, i);
		res += ((uint64_t)temp << ((uint64_t)i * 4));
	}
	permutation(res, HALF_BLOCK_SIZE, f_final_permutation_array);
	return (uint32_t)(res);
}

void init_C0_and_D0(uint64_t& key, uint32_t& C0, uint32_t& D0)
{
	append_key_to_odd(key);
	//???where we are using this append???(used for correctness checking, but not here)
	permutation(key, BLOCK_SIZE, key_permutation_array);
	take_7bits(key);
	// because we have 28 significant bits, and uint32_t has 32
	C0 = (key & ((uint64_t)0xfffffff << 28)) >> 28;
	D0 = key & 0xfffffff;
}

void update_key(uint64_t& key, uint32_t& C, uint32_t& D, int round, bool encrypt)
{
	if (encrypt)
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
	uint64_t res = transformation(CD, FINAL_KEY_SIZE, key_selection_array);
	key = res;
	//std::cout << *key << std::endl;
}

/*
	encrypts block of data(64 bits)
	Li - 32 most significant bits,
	Ri - 32 least significant bits,
	Ki - key
*/
void _encrypt(uint64_t& block, uint64_t& key, bool encrypt)
{
	permutation(block, BLOCK_SIZE, initial_permutation_array);
	uint32_t Li = ((block & ((uint64_t)0xffffffff << 32)) >> 32);
	uint32_t Ri = block & 0xffffffff;
	uint64_t ki = key;
	uint32_t Ci = 0;
	uint32_t Di = 0;
	init_C0_and_D0(key, Ci, Di);
	const int ROUNDS = 16;
	for (int i = 0; i < ROUNDS; ++i)	//16 rounds of encrypting
	{
		update_key(ki, Ci, Di, i, encrypt);
		uint32_t ltemp = Li;
		if (encrypt)
		{
			Li = Ri;
			Ri = modulo2_addition(ltemp, feistel(Ri, ki, i, encrypt));
		}
		else    //decrypt
		{
			Li = modulo2_addition(Ri, feistel(Li, ki, i, encrypt));
			Ri = ltemp;
		}
	}
	block = ((uint64_t)Li << 32) + Ri;
	permutation(block, BLOCK_SIZE, final_permutation_array);
}


uint64_t encrypt(uint64_t block, uint64_t key)
{
	_encrypt(block, key, true);
	return block;
}

uint64_t decrypt(uint64_t block, uint64_t key)
{
	_encrypt(block, key, false);
	return block;
}




