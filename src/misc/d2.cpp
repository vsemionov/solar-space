
#include <iostream>
#include <stdio.h>
#include <windows.h>

using namespace std;


void usage()
{
	cerr << "Usage: d2 dir file" << endl;
}

int make_d2(string dir, string file)
{
	FILE *fout = fopen(file.c_str(), "wb");

	fclose(fout);
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		usage();
		return 1;
	}

	return make_d2(argv[1], argv[2]);
}
