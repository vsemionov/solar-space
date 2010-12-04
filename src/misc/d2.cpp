
#include <string>
#include <list>
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <stdint.h>

using std::string;
using std::list;
using std::cout;
using std::cerr;
using std::endl;


#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif


#define HEADER_LEN 8
#define NUM_ENTRIES 512
#define ENTRY_SIZE (12 + 4 + 4)
#define START_OFFSET (HEADER_LEN + NUM_ENTRIES * ENTRY_SIZE)


typedef struct
{
	string name;
	uint32_t offset;
	uint32_t size;
} d2_entry;


void usage()
{
	cerr << "Usage: d2 dir file" << endl;
}

int make_d2(string dir, string file)
{
	list<d2_entry> entries;

	bool limit_reached = false;

	WIN32_FIND_DATA wfd;
	uint64_t offset = START_OFFSET;
	HANDLE h = FindFirstFile((dir + "\\*").c_str(), &wfd);
	BOOL b = (h != INVALID_HANDLE_VALUE);
	while (b)
	{
		if ((wfd.dwFileAttributes & (FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_DIRECTORY)) == FILE_ATTRIBUTE_ARCHIVE)
		{
			if (entries.size() >= 512)
			{
					cerr << "512 file limit reached; skipping file: " << wfd.cFileName << endl;
					limit_reached = true;
			}
			else if (wfd.nFileSizeHigh)
			{
				cerr << "skipping file larger than 4GB: " << wfd.cFileName << endl;
					limit_reached = true;
			}
			else if (offset + wfd.nFileSizeLow > 0x100000000)
			{
				cerr << "4GB file limit reached; skipping file: " << wfd.cFileName << endl;
					limit_reached = true;
			}
			else
			{
				string name = wfd.cAlternateFileName;
				if (name.empty())
					name = wfd.cFileName;
				d2_entry entry = {name, (uint32_t)offset, wfd.nFileSizeLow};
				entries.push_back(entry);
				offset += wfd.nFileSizeLow;
			}
		}
		b = FindNextFile(h,&wfd);
	}
	FindClose(h);

	if (limit_reached)
		return 1;

	d2_entry empty = {"", 0, 0};
	while (entries.size() < 512)
		entries.push_back(empty);

	FILE *fout = fopen(file.c_str(), "wb");

	char header[HEADER_LEN] = {'v', 's', ' ', 'd', '2', 0, 0, 0};
	fwrite(header, 1, sizeof(header), fout);

	for (list<d2_entry>::const_iterator it = entries.begin(); it != entries.end(); ++it)
	{
		char name[12];
		strncpy(name, it->name.c_str(), sizeof(name));
		fwrite(name, 1, sizeof(name), fout);
		fwrite(&it->offset, 1, sizeof(it->offset), fout);
		fwrite(&it->size, 1, sizeof(it->size), fout);
	}

	for (list<d2_entry>::const_iterator it = entries.begin(); it != entries.end(); ++it)
	{
		if (it->name.empty())
			continue;

		string name = dir + "\\" + it->name;
		FILE *fin = fopen(name.c_str(), "rb");
		if (!fin)
		{
			cerr << "error opening file: " << name << endl;
		}

		char buff[32768];
		int nleft = it->size;
		while (nleft > 0)
		{
			int req = min(nleft, sizeof(buff));
			int nread = fread(buff, 1, req, fin);
			if (nread != req)
			{
				if (feof(fin))
					cerr << "unexpected end of file in file: " << name << endl;
				else if (ferror(fin))
					cerr << "error reading file: " << name << endl;
				else
					cerr << "unknown error reading file: " << name << endl;
				fclose(fin);
				fclose(fout);
				return 2;
			}
			int nwritten = fwrite(buff, 1, nread, fout);
			if (nwritten != nread)
			{
				cerr << "error writing file: " << file << endl;
				fclose(fin);
				fclose(fout);
				return 3;
			}
			nleft -= nwritten;
		}

		fread(buff, 1, 1, fin);
		if (!feof(fin))
		{
			cerr << "file size has changed; file: " << name << endl;
			fclose(fin);
			fclose(fout);
			return 4;
		}

		fclose(fin);

		cout << "added file: " << name << endl;
	}

	fclose(fout);

	return 0;
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
