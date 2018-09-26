#include "stdafx.h"
#include "destechtools.h"

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

uint64_t generate_random64()
{
    return ((uint64_t)rand() << 56) + ((uint64_t)rand() << 48) + ((uint64_t)rand() << 40) + ((uint64_t)rand() << 32) +
        ((uint64_t)rand() << 24) + ((uint64_t)rand() << 16) + ((uint64_t)rand() << 8) + ((uint64_t)rand());
}

/*
Compares files byteswise, returns equality flag
Exclude last zeros is flag that one of the files may be aligned with zeros in the end
*/
bool are_files_equal(const std::string& fname1, const std::string& fname2, bool exclude_last_zeros)
{
    const int BLOCK_SIZE = 1024 * 1024;
    std::ifstream ifs1;
    std::ifstream ifs2;
    ifs1.open(fname1, std::ios_base::binary);
    ifs2.open(fname2, std::ios_base::binary);
    if (!ifs1 || !ifs2)
    {
        throw(std::runtime_error("compare_files(): can not open files"));
    }

    // checking file sizes
    // we don't check it if we are including last zeros
    if (!exclude_last_zeros)
    {
        ifs1.seekg(0, std::ios_base::end);
        ifs2.seekg(0, std::ios_base::end);
        if (ifs1.tellg() != ifs2.tellg())
        {
            return false;
        }
        ifs1.seekg(0, std::ios_base::beg);
        ifs2.seekg(0, std::ios_base::beg);
    }

    //comparing block_by_block
    std::unique_ptr<char[]> block1(new char[BLOCK_SIZE]);
    std::unique_ptr<char[]> block2(new char[BLOCK_SIZE]);
    if (exclude_last_zeros)
    {
        memset(block1.get(), 0, BLOCK_SIZE);
        memset(block2.get(), 0, BLOCK_SIZE);
    }

    ifs1.read(block1.get(), BLOCK_SIZE);
    ifs2.read(block2.get(), BLOCK_SIZE);
    while (ifs1.gcount() != 0)		//file size are equal, so checking only one stream
    {
        // if returned non-zero - files are not equal
        int err = memcmp(block1.get(), block2.get(), ifs1.gcount());
        if (err)
        {
            return false;
        }
        if (exclude_last_zeros)
        {
            memset(block1.get(), 0, BLOCK_SIZE);
            memset(block2.get(), 0, BLOCK_SIZE);
        }
        ifs1.read(block1.get(), BLOCK_SIZE);
        ifs2.read(block2.get(), BLOCK_SIZE);
    }
    return true;
}
