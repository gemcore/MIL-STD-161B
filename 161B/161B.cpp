// 161B.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

//extern "C" int bc(int argc, char *argv[]);
//extern "C" int bch(int argc, char *argv[]);
extern "C" int fax(int argc, char *argv[]);


void fax_enc(void)
{
	int argc = 4;
	char *argv[4] =
	{
		"fax",
		"I",
		"-oI.O",
		"-e"
	};
	fax(argc, argv);
}

void fax_dec(void)
{
	int argc = 3;
	char *argv[4] =
	{
		"fax",
		"I.O",
		"-oI.D"
	};
	fax(argc, argv);
}

int main()
{
	//fax_enc();
	fax_dec();
	return 0;
}

