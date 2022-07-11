#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <iostream>
#include <unistd.h>

#include "memlab.h"

using namespace std;

int main() {
	createMem(500);
	initialiseScope();
	gc_initialize();
	int varint = createVar("varint", 0);
	int varchar = createVar("varchar", 1);
	int varbool = createVar("varbool", 3);
	int varmint = createVar("varrmint", 2);
	int vararrint = createArr("vararrint", 0, 10);
	int vararrchar = createArr("vararrchar", 1, 10);
	int vararrbool = createArr("vararrbool", 3, 10);
	int vararmint = createArr("vararmint", 2, 10);

	assignVar(varint, 10);
	// int* pointer = page_table[varint / 4];
	// cout << *pointer << endl;

	assignVar(varchar, 'F');
	// int* pointerchar = page_table[varchar / 4];
	// cout << *(char*)pointerchar << endl;

	assignVar(varbool, true);
	// int* pointerbool = page_table[varbool / 4];
	// cout << *(bool*)pointerbool << endl;

	assignArr(vararrint, 99);
	// int* pointer2 = page_table[vararrint / 4];
	// cout << *(pointer2 + 4) << endl;

	assignArr(vararrchar, 'Y');
	// int* pointerx = page_table[vararrchar / 4];
	// cout << (char*)(pointerx + 4) << endl;

	assignArr(vararrbool, false);
	// int* pointery = page_table[vararrbool / 4];
	// cout << *(bool*)(pointery + 15) << endl;

	assignVar(varmint, medium_int(-(1 << 4)));
	// int* pointerz = page_table[varmint / 4];
	// cout << ((medium_int*)pointerz)->to_int() << endl;

	freeElem(varint);
	freeElem(varchar);
	freeElem(vararrint);
	printFreeSpace(100);

	compaction();

	printFreeSpace(100);

	freeElem(varbool);

	// pointerz = page_table[vararmint / 4];
	// cout << ((medium_int*)(pointerz + 1))->to_int() << endl;

	endScope();

	cleanExit();
	return 0;
}