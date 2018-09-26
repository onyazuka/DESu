#pragma once
#include <stdint.h>
#include <string>

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long long ull;
/*
generates 56 bits password from first 7 bytes of entered string
state codes:
0 - all ok,
1 - pass too short
2 - pass too long
*/
uint64_t password_from_string_to_uint64(const std::string& password, int& state)
{
	uint64_t res = 0;
	if (password.size() == 7)
	{
		state = 0;
	}
	else if (password.size() < 7)
	{
		state = 1;
	}
	else
	{
		state = 2;
	}
	for (int i = 0; i < password.size(); ++i)
	{
		res += (uint64_t)(password[i]) << ((6 - i) * 8);
	}
	return res;
}
