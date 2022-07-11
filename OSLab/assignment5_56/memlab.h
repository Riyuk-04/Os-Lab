#ifndef MY_MEMORY_MANAGEMENT_UNIT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <iostream>
#include <unistd.h>

using namespace std;

#define MAX_PAGE_TABLE 1000
#define MAX_HOLE 1000
#define VAR_NAME_SIZE 20
#define STACK_SIZE 100
#define MAX_VAR 100

#define INT 0
#define CHAR 1
#define MEDIUM_INT 2
#define BOOLEAN 3


class medium_int {
public:
	unsigned char value[3];
	medium_int(int x);
	int to_int();
};

class Variable {
public:
	string name;
	int type;
	int local_addr;
	int arr_length;
	bool isMarked;
	bool isFreed;

	Variable();
	Variable(string name, int type);
};
class Stack {
public:
	Variable* stck[STACK_SIZE];
	int index;
	Stack();
	void push(Variable* x);
	void pop();
	Variable* top();
	bool isEmpty();
};
void createMem(int size);
int createVar(string name, int type);
void assignVar(int local_addr, int valToAssign) ;
void assignVar(int local_addr, char valToAssign);
void assignVar(int local_addr, bool valToAssign);

void assignVar(int local_addr, medium_int valToAssign) ;
int createArr(string name, int type, int length) ;
void assignArr(int local_addr, int valToAssign);
int readInt(int local_addr);
void assignArr(int local_addr, char valToAssign);
int readArr(int local_addr, int index);
void writeArr(int local_addr, int index, int valToAssign);
void assignArr(int local_addr, bool valToAssign);
void assignArr(int local_addr, medium_int valToAssign);
void freeElem(int local_addr) ;
void initialiseScope();
void mark() ;
void sweep();
void compaction();
void endScope() ;
void *gc_run(void *arg);
void gc_initialize() ;
void printFreeSpace(int limit);
void cleanExit() ;

#endif
