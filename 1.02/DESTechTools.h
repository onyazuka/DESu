#pragma once
#include <stdint.h>
#include <string>
#include <random>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>

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
uint64_t password_from_string_to_uint64(const std::string& password, int& state);
uint64_t generate_random64();
bool are_files_equal(const std::string& fname1, const std::string& fname2, bool exclude_last_zeros = false);