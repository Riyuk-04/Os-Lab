#include <stdio.h>
#include <unistd.h>
#include <time.h>

void delay(int number_of_seconds)
{
	// Converting time into milli_seconds
	int milli_seconds = 1000 * number_of_seconds;

	// Storing start time
	clock_t start_time = clock();

	// looping till required time is not achieved
	while (clock() < start_time + milli_seconds)
		;
}

int main(int argc, char const *argv[])
{
	int n = 5;
	// scanf("%d", &n);
	// sleep(3);
	// for (int i = 0; i < n; ++i)
	// {
	// 	printf("%d ", i);
	// }
	// printf("\n");
	int i = 0;

	printf("%d", i++);

	sleep(5);


	return 0;
}