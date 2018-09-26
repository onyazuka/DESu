#pragma once
#include <array>
#include <bitset>
#include <string>
#include <iostream>
#include <memory>
#include <stdint.h>

/*------------------------------------------------------------------------------------------------------------*/

static const int BLOCK_SIZE = 64;
static const int HALF_BLOCK_SIZE = 32;
static const int EXPANDED_HALF_BLOCK_SIZE = 48;
static const int KEY_SIZE = 56;
static const int EXPANDED_KEY_SIZE = 64;
static const int FINAL_KEY_SIZE = 48;

typedef unsigned char uchar;
typedef unsigned long long ull;
typedef unsigned short ushort;
typedef std::array<std::array<int, 16>, 4> sub_table;
typedef std::array<const sub_table*, 8> transformation_table;

/*------------------------------------------------------------------------------------------------------------*/

// bit 7 of initial block becomes bit 1 of output block(LSB)
static const std::array<int, BLOCK_SIZE> initial_permutation_array
{ 57, 49, 41, 33, 25, 17, 9, 1,
59, 51, 43, 35, 27, 19, 11, 3,
61, 53, 45, 37, 29, 21, 13, 5,
63, 55, 47, 39, 31, 23, 15, 7,
56, 48, 40, 32, 24, 16, 8,  0,
58, 50, 42, 34, 26, 18, 10, 2,
60, 52, 44, 36, 28, 20, 12, 4,
62, 54, 46, 38, 30, 22, 14, 6 };

static const std::array<int, EXPANDED_HALF_BLOCK_SIZE> expanding_array
{
	31,0, 1, 2, 3, 4,
	3, 4, 5, 6, 7, 8,
	7, 8, 9, 10,11,12,
	11,12,13,14,15,16,
	15,16,17,18,19,20,
	19,20,21,22,23,24,
	23,24,25,26,27,28,
	27,28,29,30,31,0
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
	15,6, 19,20,28,11,27,16,
	0, 14,22,25,4, 17,30,9,
	1, 7, 23,13,31,26,2, 8,
	18,12,29,5, 21,10,3, 24
};

static const std::array<int, 64> key_permutation_array
{
	63,56,48,40,32,24,16, 8,55,0, 57,49,41,33,25,17,
	47,9, 1, 58,50,42,34,26,39,18,10,2, 59,51,43,35,
	31,62,54,46,38,30,22,14,23,6, 61,53,45,37,29,21,
	15,13,5, 60,52,44,36,28,7, 20,12,4, 27,19,11,3
};

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
	13,16,10,23,0, 4, 2, 27,14,5, 20,9, 22,18,11,3,
	25,7, 15,6, 26,19,12,1, 40,51,30,36,46,54,29,39,
	50,44,32,47,43,48,38,55,33,52,45,41,49,35,28,31
};

static const std::array<int, BLOCK_SIZE> final_permutation_array
{
	39,7, 47,15,55,23,63,31,38,6, 46,14,54,22,62,30,
	37,5, 45,13,53,21,61,29,36,4, 44,12,52,20,60,28,
	35,3, 43,11,51,19,59,27,34,2, 42,10,50,18,58,26,
	33,1, 41,9 ,49,17,57,25,32,0, 40,8, 48,16,56,24
};

static const std::array<uint64_t, BLOCK_SIZE> lshift_table
{
	(uint64_t)1 << 0,  (uint64_t)1 << 1,  (uint64_t)1 << 2,  (uint64_t)1 << 3,  (uint64_t)1 << 4,  (uint64_t)1 << 5,  (uint64_t)1 << 6,  (uint64_t)1 << 7,
	(uint64_t)1 << 8,  (uint64_t)1 << 9,  (uint64_t)1 << 10, (uint64_t)1 << 11, (uint64_t)1 << 12, (uint64_t)1 << 13, (uint64_t)1 << 14, (uint64_t)1 << 15,
	(uint64_t)1 << 16, (uint64_t)1 << 17, (uint64_t)1 << 18, (uint64_t)1 << 19, (uint64_t)1 << 20, (uint64_t)1 << 21, (uint64_t)1 << 22, (uint64_t)1 << 23,
	(uint64_t)1 << 24, (uint64_t)1 << 25, (uint64_t)1 << 26, (uint64_t)1 << 27, (uint64_t)1 << 28, (uint64_t)1 << 29, (uint64_t)1 << 30, (uint64_t)1 << 31,
	(uint64_t)1 << 32, (uint64_t)1 << 33, (uint64_t)1 << 34, (uint64_t)1 << 35, (uint64_t)1 << 36, (uint64_t)1 << 37, (uint64_t)1 << 38, (uint64_t)1 << 39,
	(uint64_t)1 << 40, (uint64_t)1 << 41, (uint64_t)1 << 42, (uint64_t)1 << 43, (uint64_t)1 << 44, (uint64_t)1 << 45, (uint64_t)1 << 46, (uint64_t)1 << 47,
	(uint64_t)1 << 48, (uint64_t)1 << 49, (uint64_t)1 << 50, (uint64_t)1 << 51, (uint64_t)1 << 52, (uint64_t)1 << 53, (uint64_t)1 << 54, (uint64_t)1 << 55,
	(uint64_t)1 << 56, (uint64_t)1 << 57, (uint64_t)1 << 58, (uint64_t)1 << 59, (uint64_t)1 << 60, (uint64_t)1 << 61, (uint64_t)1 << 62, (uint64_t)1 << 63
};

/*------------------------------------------------------------------------------------------------------------*/

/*
	Accepts 64 bit block of data, 56 bit key and working mode(0 - decrypt, 1 - encrypt)
*/
class DESEncrypter
{
public:
	enum Mode {DECRYPT = 0, ENCRYPT};
	DESEncrypter(uint64_t block_, uint64_t key_, int mode_)
		: block{ block_ }, key{ key_ }, mode{ mode_ } 
	{
		if (mode < 0 || mode > 1)
		{
			throw(std::runtime_error{ "DESEncrypter(): mode must be 1(encrypt) or 0(decrypt)" });
		}
	}
	uint64_t run();
private:
	void append_key_to_odd();
	void take_7bits();
	void narrow_block(uchar& B, int table_num);
	void fill_6bits_blocks(ull& number, std::array<uchar, 8>& blocks);
	uint32_t feistel(uint32_t& ri_minus1, int round);
	void init_C0_and_D0(uint32_t& C0, uint32_t& D0);
	void update_key(uint32_t& C, uint32_t& D, int round);

	const int mode;
	uint64_t block;
	uint64_t key;
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
	// !!!assuming uint64_t is biggest type used!!!
	uint64_t mask = ((uint64_t)1 << digits) - 1;
	if (digits == -1)
	{
		digits = sizeof(UINT) * CHAR_BIT;
		mask = ((uint64_t)1 << digits) - 1;
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
	// !!!assuming uint64_t is biggest type used!!!
	uint64_t mask = ((uint64_t)1 << digits) - 1;
	if (digits == -1)
	{
		digits = sizeof(UINT) * CHAR_BIT;
		mask = ((uint64_t)1 << digits) - 1;
	}
	num &= mask;
	return (num >> val) | (num << (digits - val)) & mask;
}


/*
	Function expanding or narrowing or permutating block of data.
	All changes are done inside passed block
	NOTE: bit 1 is always most significant bit
*/
template<typename T>
void transformation(uint64_t& block, int size, T& transform_array)
{
	// 64bits = 8bytes, can be represented as ull
	uint64_t block_copy = block;
	uint64_t res = 0;
	uint64_t temp = 0;
	for (int i = 0; i < size; ++i)
	{
		temp = (block_copy & lshift_table[transform_array[size - 1 - i]]);
		if (temp)
		{
			res |= lshift_table[i];
		}
	}
	block = res;
}

/*
	Addition modulo 2 of two 64 bits numbers(blocks), which in fact may be 48 bits expanded half-blocks
*/
uint64_t modulo2_addition(uint64_t num1, uint64_t num2);







