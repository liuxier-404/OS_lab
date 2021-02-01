#include <stdio.h>
#include <stdlib.h>
#include<unistd.h>
int main(int argc,char** argv){
	printf("program2 sun succeed!\n");
	int time=atoi(argv[1]);
	sleep(time);
	return 0;
}
