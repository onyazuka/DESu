// DES.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DES.h"




int main()
{
	ull number1 = 18446744041709521813;
	uchar* uch1 = reinterpret_cast<uchar*>(&number1);
	permutation(uch1, 64, initial_permutation_array);
    return 0;
}

