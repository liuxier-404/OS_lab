#include<Windows.h>
#include<stdio.h>
#include<stdlib.h>
int main(int argc, char* argv[]) {
	int time = atoi(argv[1]);
	int loop = 12000;
	int sum = 0;
	for (int i = 0; i < loop; i++) {
		sum += i;
	}
	printf("program2 run succeed!\n");
	Sleep(1000 * time);
	return 0;
}