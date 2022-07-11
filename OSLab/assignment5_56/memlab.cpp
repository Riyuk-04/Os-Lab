#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <iostream>
#include <unistd.h>
#include "memlab.h"

using namespace std;

int localAddress = 0;
int gc_footprint = 0;

medium_int::medium_int(int x) {
	// cout << "FIWIF";
	int r = (1 << 8) - 1;
	if (x >= (1 << 23) || x <= (-(1 << 23))) {
		cout << "OVERFOLOW: Value is more than 3 bytes" << endl;
	}
	// cout << x;

	this->value[2] = (char)(x & r);
	x = x >> 8;
	this->value[1] = (char)(x & r);
	x = x >> 8;
	this->value[0] = (char)(x & r);
	x = x >> 8;
	// cout << endl << (int)value;
}

int medium_int::to_int()
{

	if (this->value[0] >> 7) {
		// cout << "YES";
		return ( (((1 << 8) - 1) << 24) | ((this->value[0]) << 16) | ((this->value[1]) << 8) | (this->value[2]));
	}
	return ((this->value[0]) << 16) | ((this->value[1]) << 8) | (this->value[2]);
}



Variable::Variable() {
	// name = (char *)malloc(sizeof(char) * VAR_NAME_SIZE);
	type = -1;
}

Variable::Variable(string name, int type) {

	this->name = name;
	this->type = type;
	this->local_addr = localAddress;
	this->arr_length = 1;
	this->isMarked = false;
	this->isFreed = false;
}




Stack::Stack() { index = -1; }

void Stack:: push(Variable* x) {
	if (index == STACK_SIZE - 1) {
		cout << "Error, stack overFlow!" << endl;
		exit(1);
	}
	stck[++index] = x;
}

void Stack::pop() {
	if (index == -1) {
		cout << "Error, stack is Empty!" << endl;
		exit(1);
	}
	index--;
}

Variable* Stack::top() { return stck[index]; }

bool Stack::isEmpty() {
	if (index == -1) return 1;
	return 0;
}


bool garbageFlag = false;
int mem_size;
int* physical_memory;
int* page_table[MAX_PAGE_TABLE];
Stack globalStack = Stack();
Variable var_array[MAX_VAR];
bool* freespace;
pthread_mutex_t memLock;

string typeFromInt(int x) {
	if (x == 0)	return "INT";
	if (x == 1)	return "CHAR";
	if (x == 2)	return "MEDIUM INT";
	if (x == 3)	return "BOOLEAN";
}

void createMem(int size) {
	mem_size = size;
	if (pthread_mutex_init(&memLock, NULL) != 0) {
		printf("\n mutex init has failed\n");
	}
	pthread_mutex_lock(&memLock);
	physical_memory = (int*)malloc(size);
	freespace = (bool *)malloc(size / 4);
	for (int i = 0; i < size / 4; ++i)
		freespace[i] = true;
	pthread_mutex_unlock(&memLock);
	cout << "MEMORY CREADTED : " << size << " bytes allocated\n";
}

int createVar(string name, int type) {
	pthread_mutex_lock(&memLock);
	Variable var = Variable(name, type);
	var_array[localAddress / 4] = var;
	globalStack.push(&var_array[localAddress / 4]);
	// cout << (&var_array[localAddress / 4])->isMarked << "ffa";

	//Assigning physical address
	for (int i = 0; i < mem_size / 4; ++i) {
		if (freespace[i]) {
			freespace[i] = false;
			if (localAddress / 4 >= MAX_PAGE_TABLE) {
				cout << "PAGE TABLE FULL";
				exit(1);
			}
			page_table[localAddress / 4] = physical_memory + i;
			break;
		}
		if (i == mem_size / 4 - 1) {
			cout << "MEMORY FULL";
			exit(1);
		}
	}

	cout << "Variable created->Name:" << name << "\tType:" << typeFromInt(type) << "\tPhysical address:" << page_table[localAddress / 4] << "\n";
	cout << "Page tabe enty created: Adrdress" << page_table[localAddress / 4] << endl;
	localAddress += 4;
	gc_footprint += 1;
	pthread_mutex_unlock(&memLock);
	return (localAddress - 4);
}

void assignVar(int local_addr, int valToAssign) {
	if (var_array[local_addr / 4].type != 0) {
		cout << "Please assign an int value\n";
		return;
	}
	pthread_mutex_lock(&memLock);
	int* pointer = page_table[local_addr / 4];
	*pointer = valToAssign;
	pthread_mutex_unlock(&memLock);
	cout << "Variable assigned->Name:" << var_array[local_addr / 4].name << "\tType:" << typeFromInt(var_array[local_addr / 4].type) << "\tValue Assigned:" << valToAssign << endl;
}

int readInt(int local_addr) {
	return *(page_table[local_addr / 4]);
}

void assignVar(int local_addr, char valToAssign) {
	if (var_array[local_addr / 4].type != 1) {
		cout << "Please assign a char\n";
		return;
	}
	pthread_mutex_lock(&memLock);
	int* pointer = page_table[local_addr / 4];
	char* pointerChar = (char*)pointer;
	*pointerChar = valToAssign;
	pthread_mutex_unlock(&memLock);
	cout << "Variable assigned->Name:" << var_array[local_addr / 4].name << "\tType:" << typeFromInt(var_array[local_addr / 4].type) << "\tValue Assigned:" << valToAssign << endl;
}

void assignVar(int local_addr, bool valToAssign) {
	if (var_array[local_addr / 4].type != 3) {
		cout << "Please assign a bool0\n";
		return;
	}
	pthread_mutex_lock(&memLock);
	int* pointer = page_table[local_addr / 4];
	bool* pointerbool = (bool*)pointer;
	*pointerbool = valToAssign;
	pthread_mutex_unlock(&memLock);
	cout << "Variable assigned->Name:" << var_array[local_addr / 4].name << "\tType:" << typeFromInt(var_array[local_addr / 4].type) << "\tValue Assigned:" << valToAssign << endl;
}

void assignVar(int local_addr, medium_int valToAssign) {
	if (var_array[local_addr / 4].type != 2) {
		cout << "Please assign a medium int\n";
		return;
	}
	pthread_mutex_lock(&memLock);
	int* pointer = page_table[local_addr / 4];
	medium_int* pointermedium_int = (medium_int*)pointer;
	*pointermedium_int = valToAssign;
	pthread_mutex_unlock(&memLock);
	cout << "Variable assigned->Name:" << var_array[local_addr / 4].name << "\tType:" << typeFromInt(var_array[local_addr / 4].type) << "\tValue Assigned:" << valToAssign.to_int() << endl;
}


int createArr(string name, int type, int length) {
	pthread_mutex_lock(&memLock);
	Variable var = Variable(name, type);
	var.arr_length = length;
	var_array[localAddress / 4] = var;
	globalStack.push(&var_array[localAddress / 4]);

	//Assigning physical address
	int flag = length;
	int index = -1;
	for (int i = 0; i < mem_size / 4; ++i) {
		if (freespace[i]) {
			flag--;
			if (flag == 0) {
				index = i;
			}
		} else {
			flag = length;
		}
	}
	if (index == -1) {
		cout << "No free space found : Calling Compaction\n";
		compaction();
	}

	if (localAddress / 4 >= MAX_PAGE_TABLE) {
		cout << "PAGE TABLE FULL";
		exit(1);
	}
	page_table[localAddress / 4] = physical_memory + (index - length + 1);
	for (int i = index - length + 1; i <= index; ++i) {
		freespace[i] = false;
	}

	cout << "Array created->Name:" << name << "\tType:" << typeFromInt(type) << "\tSize:" << length << "\tPhysical address:" << page_table[localAddress / 4] << "\n";
	cout << "Page tabe enty created: Adrdress" << page_table[localAddress / 4] << endl;
	localAddress += 4;
	gc_footprint += length;
	pthread_mutex_unlock(&memLock);
	return (localAddress - 4);
}

void assignArr(int local_addr, int valToAssign) {
	if (var_array[local_addr / 4].type != 0) {
		cout << "Please assign an int value\n";
		return;
	}
	pthread_mutex_lock(&memLock);
	int length = var_array[local_addr / 4].arr_length;
	int* pointer = page_table[local_addr / 4];
	for (int i = 0; i < length; ++i)
	{
		*(pointer + i) = valToAssign;
	}
	pthread_mutex_unlock(&memLock);
	cout << "Array assigned->Name:" << var_array[local_addr / 4].name << "\tType:" << typeFromInt(var_array[local_addr / 4].type) << "\tValue Assigned:" << valToAssign << endl;
}

int readArr(int local_addr, int index) {
	if (index >= var_array[local_addr / 4].arr_length) {
		cout << "Segmentation Fault (Reading ahead of an array)";
		exit(1);
	}
	return *(page_table[local_addr / 4] + index);
}

void writeArr(int local_addr, int index, int valToAssign) {
	if (index >= var_array[local_addr / 4 ].arr_length) {
		cout << "Segmentation Fault (Writing ahead of an array)";
		exit(1);
	}
	*(page_table[local_addr / 4] + index) = valToAssign;
}

void assignArr(int local_addr, char valToAssign) {
	if (var_array[local_addr / 4].type != 1) {
		cout << "Please assign a char\n";
		return;
	}
	pthread_mutex_lock(&memLock);
	int length = var_array[local_addr / 4].arr_length;
	int* pointer = page_table[local_addr / 4];
	for (int i = 0; i < length; ++i)
	{
		char* pointerChar = (char*)(pointer + i);
		*pointerChar = valToAssign;
	}
	pthread_mutex_unlock(&memLock);
	cout << "Array assigned->Name:" << var_array[local_addr / 4].name << "\tType:" << typeFromInt(var_array[local_addr / 4].type) << "\tValue Assigned:" << valToAssign << endl;
}

void assignArr(int local_addr, bool valToAssign) {
	if (var_array[local_addr / 4].type != 3) {
		cout << "Please assign a bool\n";
		return;
	}
	pthread_mutex_lock(&memLock);
	int length = var_array[local_addr / 4].arr_length;
	int* pointer = page_table[local_addr / 4];
	for (int i = 0; i < length; ++i)
	{
		bool* pointerbool = (bool*)(pointer + i);
		*pointerbool = valToAssign;
	}
	pthread_mutex_unlock(&memLock);
	cout << "Array assigned->Name:" << var_array[local_addr / 4].name << "\tType:" << typeFromInt(var_array[local_addr / 4].type) << "\tValue Assigned:" << valToAssign << endl;

}

void assignArr(int local_addr, medium_int valToAssign) {
	if (var_array[local_addr / 4].type != 2) {
		cout << "Please assign a medium int\n";
		return;
	}
	pthread_mutex_lock(&memLock);
	int length = var_array[local_addr / 4].arr_length;
	int* pointer = page_table[local_addr / 4];
	for (int i = 0; i < length; ++i)
	{
		medium_int* pointermedium_int = (medium_int*)(pointer + i);
		*pointermedium_int = valToAssign;
	}
	pthread_mutex_unlock(&memLock);
	cout << "Array assigned->Name:" << var_array[local_addr / 4].name << "\tType:" << typeFromInt(var_array[local_addr / 4].type) << "\tValue Assigned:" << valToAssign.to_int() << endl;
}

void freeElem(int local_addr) {
	pthread_mutex_lock(&memLock);
	gc_footprint -= var_array[local_addr / 4].arr_length;
	int length = var_array[local_addr / 4].arr_length;
	var_array[local_addr / 4].isMarked = true;
	var_array[local_addr / 4].isFreed = true;
	int* pointer = page_table[local_addr / 4];
	int temp = pointer - physical_memory;
	// cout << endl << temp << endl;;
	for (int i = temp; i < temp + length; ++i) {
		freespace[i] = true;
	}
	cout << "Element Freed->Name:" << var_array[local_addr / 4].name << "\tType:" << typeFromInt(var_array[local_addr / 4].type) << "\tPhysical address:" << page_table[local_addr / 4] << "\n";
	pthread_mutex_unlock(&memLock);
}

void initialiseScope() {
	pthread_mutex_lock(&memLock);
	globalStack.push(NULL);
	pthread_mutex_unlock(&memLock);
}

void mark() {
	pthread_mutex_lock(&memLock);
	cout << "Marking phase Begins\n";
	while (globalStack.top() != NULL) {
		// freeElem(globalStack.top()->local_addr);
		var_array[globalStack.top()->local_addr / 4].isMarked = true;
		globalStack.pop();
	}
	globalStack.pop();
	cout << "Marking phase Ends\n";
	pthread_mutex_unlock(&memLock);
}

void sweep() {
	// pthread_mutex_lock(&memLock);
	cout << "Sweep phase begins\n";
	for (int i = 0; i < localAddress / 4; ++i)
	{
		if (var_array[i].isMarked && !var_array[i].isFreed) {
			freeElem(var_array[i].local_addr);
		}
	}
	// pthread_mutex_unlock(&memLock);
}

void compaction() {
	pthread_mutex_lock(&memLock);
	for (int i = 0; i < mem_size / 4; ++i)
	{
		if (freespace[i]) {
			int j = i;
			while (j < mem_size / 4) {
				if (freespace[j] == 0) {
					break;
				}
				j++;
			}
			if (j == mem_size / 4) {
				cout << "Compacted\n";
				pthread_mutex_unlock(&memLock);
				return;
			}
			for (int x = 0; x < MAX_PAGE_TABLE; ++x) {
				if (page_table[x] == physical_memory + j) {
					cout << "Compacting variable\tName:" << var_array[x].name << "\tPhysical address:" << page_table[x] << endl;
					page_table[x] = physical_memory + i;
					for (int y = 0; y < var_array[x].arr_length; ++y) {
						*(physical_memory + i++) = *(physical_memory + j++);
						freespace[i - 1] = 0;
						freespace[j - 1] = 1;
					}
					i--;
					break;
				}
			}
		}
	}
	pthread_mutex_unlock(&memLock);
}

void endScope() {
	mark();
	sweep();
	compaction();
}

void *gc_run(void *arg) {
	while (1) {
		usleep(5000);
		compaction();
	}
}

void gc_initialize() {
	pthread_t garbage_thread;
	pthread_attr_t attr;
	pthread_attr_init (&attr);

	int *arg = (int *) malloc(sizeof(*arg));
	*arg = 1;

	pthread_create(&garbage_thread, &attr, gc_run, arg);
	// pthread_join(garbage_thread, NULL);
}

void printFreeSpace(int limit) {
	cout << "FREE SPACE ARRAY:\n";
	for (int i = 0; i < limit; ++i)
	{
		cout << freespace[i];
	}
	cout << endl;
}

void cleanExit() {
	cout << "-------------------------------------------------------- Clean Exit --------------------------------------------------------\n";
	exit(1);
}