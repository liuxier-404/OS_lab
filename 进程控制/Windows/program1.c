#include<stdio.h>
#include<stdlib.h>
int main(int argc,char* argv[]) {
	int loop = 12000;
	int sum = 1;
	for (int j = 1; j< loop; j++) {
		for (int i = 1; i < loop; i++) {
			sum *= i;
			sum %= 10000;
			sum++;
		}
	}
	printf("%d\n", sum);
	printf("program1 run succeed!\n");
	return 0;
}