#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <iostream>
#include <unistd.h>
#include "memlab.h"

using namespace std;

int fibonacciProduct(int k) {
	initialiseScope();

	int prod = createVar("prod", INT);
	assignVar(prod, 1);

	int arr = createArr("arr", INT, readInt(k));
	assignArr(arr, 1);

	for (int i = 2; i < readInt(k); ++i) {
		writeArr(arr, i, readArr(arr, i - 2) + readArr(arr, i - 1));
		assignVar(prod, readInt(prod) * readArr(arr, i));
	}

	int ans = readInt(prod);
	endScope();
	return ans;
}

int main() {
	createMem(250e6);
	initialiseScope();
	gc_initialize();

	int k = createVar("k", INT);
	assignVar(k, 10);

	int ans = fibonacciProduct(k);

	cout << "The Fibinaci Product is : " << ans << endl;

	endScope();
	cleanExit();
	return 0;
}