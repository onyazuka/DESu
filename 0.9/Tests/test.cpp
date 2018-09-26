#include "pch.h"
#include "../DES/DES.h"
#include <ctime>
#include <random>

typedef unsigned char uchar;
typedef unsigned long long ull;
typedef unsigned int uint;

TEST(PermutationTest, DESTest)
{
	//0b1111111111111111111111111111100010001100101001100100101110010101
	uint64_t number1 = 18446744041709521813;
	//0b1110010101001001000001101010101101111001011100101110000101100100
	uint64_t number2 = 16521744041532121444;
	transformation(number1, 64, initial_permutation_array);
	transformation(number2, 64, initial_permutation_array);

	EXPECT_FALSE(number1 & ((uint64_t)1 << 0));
	EXPECT_TRUE(number1 & ((uint64_t)1 << 1));
	EXPECT_FALSE(number1 & ((uint64_t)1 << 2));
	EXPECT_FALSE(number1 & ((uint64_t)1 << 17));
	EXPECT_FALSE(number2 & ((uint64_t)1 << 57));
	EXPECT_TRUE(number2 & ((uint64_t)1 << 16));
	EXPECT_TRUE(number2 & ((uint64_t)1 << 0));
	EXPECT_TRUE(number1 & ((uint64_t)1 << 1));
	EXPECT_TRUE(number2 & ((uint64_t)1 << 40));
	EXPECT_TRUE(number2 & ((uint64_t)1 << 41));

	number1 = 18446744041709521813;
	uint64_t n1low = static_cast<uint32_t>(number1);
	EXPECT_EQ(n1low, 2359708565);
	transformation(n1low, 32, f_final_permutation_array);
	EXPECT_FALSE(n1low & ((uint32_t)1 << 0));
	EXPECT_TRUE(n1low & ((uint32_t)1 << 14));
}

TEST(TransformationTest, DESTest)
{
	//0b1111111111111111111111111111100010001100101001100100101110010101
	uint64_t number1 = 18446744041709521813;
	//0b1110010101001001000001101010101101111001011100101110000101100100
	uint64_t number2 = 16521744041532121444;

	// r - least significant bits, l - most significant bits
	uint64_t n1r = static_cast<uint32_t>(number1);
	EXPECT_EQ(n1r, 2359708565);
	uint64_t n1l = static_cast<uint32_t>(number1 >> 32);
	EXPECT_EQ(n1l, 4294967288);
	uint64_t n2r = static_cast<uint32_t>(number2);
	EXPECT_EQ(n2r, 2037571940);
	uint64_t n2l = static_cast<uint32_t>(number2 >> 32);
	EXPECT_EQ(n2l, 3846768299);

	transformation(n1r, EXPANDED_HALF_BLOCK_SIZE, expanding_array);
	transformation(n1l, EXPANDED_HALF_BLOCK_SIZE, expanding_array);
	transformation(n2r, EXPANDED_HALF_BLOCK_SIZE, expanding_array);
	transformation(n2l, EXPANDED_HALF_BLOCK_SIZE, expanding_array);

	EXPECT_TRUE(n1r & ((uint64_t)1 << 0));
	EXPECT_TRUE(n1r & ((uint64_t)1 << 1));
	EXPECT_FALSE(n1r & ((uint64_t)1 << 2));
	EXPECT_FALSE(n1r & ((uint64_t)1 << 3));
	EXPECT_FALSE(n1r & ((uint64_t)1 << 4));
	EXPECT_TRUE(n1r & ((uint64_t)1 << 37));
	EXPECT_TRUE(n1r & ((uint64_t)1 << 35));
	EXPECT_TRUE(n1r & ((uint64_t)1 << 34));

	EXPECT_FALSE(n2l & ((uint64_t)1 << 9));
	EXPECT_TRUE(n2l & ((uint64_t)1 << 10));

	//0b11011010100110010001100011000000011001101010100011110010
	number1 = 61529876509141234;
	transformation(number1, EXPANDED_HALF_BLOCK_SIZE, key_selection_array);
	EXPECT_FALSE(number1 & ((uint64_t)1 << 32));
	EXPECT_TRUE(number1 & ((uint64_t)1 << 24));
	EXPECT_FALSE(number1 & ((uint64_t)1 << 4));

}

TEST(NarrowTest, DESTest)
{
	uchar a = 0b101111;
	narrow_block(a, 1);
	EXPECT_EQ(a, 2);
	a = 0b000110;
	narrow_block(a, 2);
	EXPECT_EQ(a, 14);
	a = 0b000000;
	narrow_block(a, 4);
	EXPECT_EQ(a, 2);
	a = 0b110011;
	narrow_block(a, 0);
	EXPECT_EQ(a, 11);
}

TEST(AppendKeyTest, DESTest)
{
	//0b11111110100011101011111001010110010101100000111000011101
	//output: 0b0111111100100011010101111110010100110010010110000001110010011101
	uint64_t key = 71651592227917341;
	append_key_to_odd(key);
	EXPECT_EQ(key, 9161262708905483421);
	
	//0b11011001000111001010110001011110010001111100011000110010
	//output: 0b1110110011000111000101010100010111110010000111111000110000110010
	key = 61111596587927090;
	append_key_to_odd(key);
	EXPECT_EQ(key, 17061629103475493938);
}

TEST(Take7BitsTest, DESTest)
{
	uint64_t key = 9161262708905483421;
	take_7bits(key);
	EXPECT_EQ(key, 71651592227917341);

	key = 17061629103475493938;
	take_7bits(key);
	EXPECT_EQ(key, 61111596587927090);
}

TEST(RolRorTest, DESTest)
{
	uchar uc1 = 129;
	EXPECT_EQ(rol(uc1, 2), 6);
	EXPECT_EQ(ror(uc1, 5), 12);
	EXPECT_EQ(ror(uc1, 10), 0);
	uint32_t i1 = 4275878570;
	EXPECT_EQ(rol(i1, 1), 4256789845);
	EXPECT_EQ(rol(i1, 0), 4275878570);
	EXPECT_EQ(ror(i1, 0), 4275878570);
	EXPECT_EQ(rol(i1, 8), 3703220990);
	EXPECT_EQ(ror(i1, 14), 3937139570);

	EXPECT_EQ(rol(i1, 7, 24), 6116718);

	uchar neko = 83;
	EXPECT_EQ(ror(neko, 1, 3), 5);

	i1 = 64321;
	EXPECT_EQ(rol(i1, 3, 12), 2573);
}

uint64_t generate_56bits_key()
{
	return ((uint64_t)rand() << 40) + ((uint64_t)rand() << 24) + ((uint64_t)rand() << 8) + (rand() >> 8);
}

uint64_t generate_64bits_block()
{
	return ((uint64_t)rand() << 48) + ((uint64_t)rand() << 32) + ((uint64_t)rand() << 16) + rand();
}

TEST(StableTest, DESTest)
{
	srand(time(0));
	int n = 10;
	for (int i = 0; i < n; ++i)
	{
		uint64_t key = generate_56bits_key();
		uint64_t block = generate_64bits_block();
		uint64_t encr = encrypt(block, key);
		uint64_t decr = decrypt(encr, key);
		EXPECT_EQ(block, decr);
	}
}

