#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <iostream>
#include <unistd.h>
#include "memlab.h"

using namespace std;
void funcInt(int x, int y) {
	initialiseScope();
	int arr = createArr("arr", 0, 50000);
	assignArr(arr, rand() % 100);
	//freeElem(arr);
	endScope();
}

void funcChar(int x, int y) {
	initialiseScope();
	int arr = createArr("arr", 1, 50000);
	assignArr(arr, 'E');
	//freeElem(arr);
	endScope();
}

void funcBool(int x, int y) {
	initialiseScope();
	int arr = createArr("arr", 3, 50000);
	assignArr(arr, true);
	//freeElem(arr);
	endScope();
}

void funcmint(int x, int y) {
	initialiseScope();
	int arr = createArr("arr", 2, 50000);
	assignArr(arr, medium_int(rand() % 100));
	//freeElem(arr);
	endScope();
}


int main() {
	createMem(250e6);
	initialiseScope();
	gc_initialize();
	cout << endl;

	int var1 = createVar("x", INT);
	int var2 = createVar("y", INT);
	funcInt(var1, var2);
	cout << "------------------------------------ARRAY 1 CREATED,ASSIGNED AND FREED--------------------------------------\n\n" << endl;
	int var3 = createVar("a", CHAR);
	int var4 = createVar("b", CHAR);
	funcChar(var3, var4);
	cout << "------------------------------------ARRAY 2 CREATED,ASSIGNED AND FREED--------------------------------------\n\n" << endl;
	int var5 = createVar("m", MEDIUM_INT);
	int var6 = createVar("n", MEDIUM_INT);
	funcmint(var5, var6);
	cout << "------------------------------------ARRAY 3 CREATED,ASSIGNED AND FREED--------------------------------------\n\n" << endl;
	int var7 = createVar("o", BOOLEAN);
	int var8 = createVar("p", BOOLEAN);
	funcBool(var7, var8);
	cout << "------------------------------------ARRAY 4 CREATED,ASSIGNED AND FREED--------------------------------------\n\n" << endl;
	int var9 = createVar("q", INT);
	int var10 = createVar("r", INT);
	funcInt(var9, var10);
	cout << "------------------------------------ARRAY 5 CREATED,ASSIGNED AND FREED--------------------------------------\n\n" << endl;
	int var11 = createVar("s", CHAR);
	int var12 = createVar("t", CHAR);
	funcChar(var11, var12);
	cout << "------------------------------------ARRAY 6 CREATED,ASSIGNED AND FREED--------------------------------------\n\n" << endl;
	int var13 = createVar("u", MEDIUM_INT);
	int var14 = createVar("v", MEDIUM_INT);
	funcmint(var13, var14);
	cout << "------------------------------------ARRAY 7 CREATED,ASSIGNED AND FREED--------------------------------------\n\n" << endl;
	int var15 = createVar("w", BOOLEAN);
	int var16 = createVar("x", BOOLEAN);
	funcBool(var15, var16);
	cout << "------------------------------------ARRAY 8 CREATED,ASSIGNED AND FREED--------------------------------------\n\n" << endl;
	int var17 = createVar("y", INT);
	int var18 = createVar("z", INT);
	funcInt(var17, var18);
	cout << "------------------------------------ARRAY 9 CREATED,ASSIGNED AND FREED--------------------------------------\n\n" << endl;
	int var19 = createVar("A", CHAR);
	int var20 = createVar("B", CHAR);
	funcChar(var19, var20);
	cout << "------------------------------------ARRAY 10 CREATED,ASSIGNED AND FREED--------------------------------------\n\n" << endl;
	endScope();
	cleanExit();
	return 0;
}