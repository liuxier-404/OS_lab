#include <stdio.h>
#include<stdlib.h>
#include <time.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include<sys/wait.h>
#include <unistd.h>

//宏定义消费者和生产者进程相关的常量
#define BUFFER_SIZE 3
#define PRODUCER_NUM 2
#define CUSTOMER_NUM 3
#define PRODUCER_TIMES 6
#define CUSTOMER_TIMES 4

//宏定义进程的键值
#define SEM_KEY 1234
#define SHM_KEY 6789

//信号量的索引值
#define SEM_FULL 1
#define SEM_EMPTY 0；
#define SEM_MUTEX 2

//缓冲区结构
struct mybuffer {
	int buffer[BUFFER_SIZE];
	int head;
	int tail;
	int isEmpty;
};

//生成一个1-9的随机数，用于输出数据和随机睡眠时间
int getRandom() {
	int t;
	srand((unsigned)(getpid() + time(NULL)));
	t = rand() % 10;
	return t;
}


//p 操作
void p(int sem_id, int sem_num) {
	struct sembuf sbf;
	sbf.sem_num = sem_num;
	sbf.sem_flg = 0;
	sbf.sem_op = -1;
	semop(sem_id, &sbf, 1);
}


//v操作
void v(int sem_id, int sem_num) {
	struct sembuf sbf;
	sbf.sem_num = sem_num;
	sbf.sem_flg = 0;
	sbf.sem_op = 1;
	semop(sem_id, &sbf, 1);
}


int main(int argc, char* argv[]) {
	//定义进程id
	pid_t pid_p;
	pid_t pid_c;
	time_t nowtime;
	//创建信号量集,返回信号标识符
	int sem_id = semget(SEM_KEY, 3, IPC_CREAT | 0660);

	if (sem_id == -1) {
		printf("信号量集创建失败\n");
		_Exit(1);
	}
	//自定义union semun结构
	union  semun
	{
		int value;
		struct semd_ds* buf;
		unsigned short int* array;
		struct seminfo* _buf;
	};

	//初始化信号量集
	union semun sem_val;
	sem_val.value = BUFFER_SIZE;
	//empty初始化为缓冲区长度
	semtcl(sem_id, SEM_EMPTY, SETVAL, sem_val);
	sem_val.value = 0;
	//初始化full为0
	semtcl(sem_id, SEM_FULL, SETVAL, sem_val);
	sem_val.value = 1;
	//初始化mutex为1
	semtcl(sem_id, SEM_MUTEX, SETVAL, sem_val);

	//创建共享内存区
	int shm_id = shmget(SHM_KEY, sizeof(struct mybuffer), 0600 | IPC_CREAT);
	if (shm_id < 0) {
		//创建失败
		printf("创建共享内存区失败\n");
		exit(1);
	}
	//初始化缓冲区,启动对该共享区的访问，并把它连接到当前进程的地址空间
	struct mybuffer* shmaddr = shmat(shm_id, 0, 0);
	if (shmaddr == (void*)-1) {
		printf("共享内存区连接失败\n");
		_Exit(1);
	}
	shmaddr->head = 0;
	shmaddr->tail = 0;
	shmaddr->isEmpty = 1;
	int data;
	int index;
	int length;
	//生产者进程
	int produce_num = 0;
	while ((produce_num++) < PRODUCER_NUM) {
		if ((pid_p = fork()) < 0) {
			printf("生产者进程创建失败\n");
			exit(1);
		}
		if (pid_p == 0) {
			//将共享内存区连接到生产者进程的地址空间上
			if ((shmaddr =static_cast<struct mybuffer*>( shmat(shm_id, 0, 0))) == (void*)-1) {
				printf("共享内存连接失败\n");
				exit(1);
			}
			//写入数据
			for (int i = 0; i < PRODUCER_TIMES; i++) {
				p(sem_id, SEM_EMPTY);//p(empty)
				int data = getRandom();
				sleep(data);
				p(sem_id, SEM_MUTEX);//p(mutex)
				shmaddr->buffer[shmaddr->tail] = data;
				shmaddr->isEmpty = 0;
				shmaddr->tail = (shmaddr->tail + 1) % BUFFER_SIZE;
				nowtime = time(NULL);
				printf("%02d时%02d分%02d秒\n", localtime(&nowtime)->tm_hour, localtime(&nowtime)->tm_min, localtime(&nowtime)->tm_sec);
				printf("生产者%d 将数据%d写入到缓存区中\n",produce_num,data);
				printf("当前缓冲区的数据：");
				//计算当前缓冲区数据的长度
				int buffer_length = (shmaddr->tail + BUFFER_SIZE - shmaddr->head) % BUFFER_SIZE;
				if (buffer_length == 0) buffer_length = 3;
				for (int i = 0; i < buffer_length; i++) {
					printf("%d\t", shmaddr->buffer[(shmaddr->head + i)%BUFFER_SIZE]);
				}
				printf("\n\n");
				fflush(stdout);//刷新缓冲区
				v(shm_id, SEM_FULL);//v(full)
				v(shm_id, SEM_MUTEX);//v(mutex)

			}
			//断开共享内存连接
			shmdt(shmaddr);
			exit(0);
		}

	}
	int cus_num = 0;
	while (cus_num++ < CUSTOMER_NUM) {
		if ((pid_c = fork()) < 0) {
			printf("消费者进程创建失败\n");
			exit(1);
		}
		//创建消费者进程
		if (pid_c == 0) {
			if ((shmaddr =static_cast<struct mybuffer*> (shmat(shm_id, 0, 0)) )== (void*)-1) {
				printf("共享内存连接失败\n");
				exit(1);
			}
			for (int i = 0; i < CUSTOMER_TIMES; i++) {//消费者读取数据
				p(shm_id, SEM_FULL);
				int sleeptime = getRandom();
				sleep(sleeptime);
				p(shm_id, SEM_MUTEX);
				data = shmaddr->buffer[shmaddr->head];
				shmaddr->head = (shmaddr->head + 1) % BUFFER_SIZE;
				shmaddr->isEmpty = (shmaddr->head == shmaddr->tail);
				nowtime = time(NULL);
				printf("当前时间：%02d时%02d分%02d秒\n", localtime(&nowtime)->tm_hour, localtime(&nowtime)->tm_min, localtime(&nowtime)->tm_sec);
				printf("消费者%d 将数据%d从缓冲区取出.\n", num_c, data);
				int bufflength = (shmaddr->tail + BUFFER_SIZE - shmaddr->head) % BUFFER_SIZE;
				if (bufflength == 0)
					printf("当前缓冲区为空！\n\n");
				else
				{
					printf("当前缓冲区内的数据：\n");
					for (int j = 0; j < bufflength; j++)
					{
						int index = (shmaddr->head + j) % BUFFER_SIZE;
						printf("%d ", shmaddr->buffer[index]);
					}
					printf("\n\n");
				}
				fflush(stdout);
				v(sem_id, SEM_MUTEX);    //解锁
				v(sem_id, SEM_EMPTY);

			}
			shmdt(shmaddr);
			exit(0);
		}
	}
	while (wait(0) != -1);
	shmdt(shmaddr);
	shmctl(shm_id, IPC_RMID, 0);
	semctl(sem_id, IPC_RMID, 0);
	printf("模拟结束！\n");
	fflush(stdout);
	exit(0);
}