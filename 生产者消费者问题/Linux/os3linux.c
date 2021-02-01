#include <stdio.h>
#include<stdlib.h>
#include <time.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include<sys/wait.h>
#include <unistd.h>

//�궨�������ߺ������߽�����صĳ���
#define BUFFER_SIZE 3
#define PRODUCER_NUM 2
#define CUSTOMER_NUM 3
#define PRODUCER_TIMES 6
#define CUSTOMER_TIMES 4

//�궨����̵ļ�ֵ
#define SEM_KEY 1234
#define SHM_KEY 6789

//�ź���������ֵ
#define SEM_FULL 1
#define SEM_EMPTY 0��
#define SEM_MUTEX 2

//�������ṹ
struct mybuffer {
	int buffer[BUFFER_SIZE];
	int head;
	int tail;
	int isEmpty;
};

//����һ��1-9�������������������ݺ����˯��ʱ��
int getRandom() {
	int t;
	srand((unsigned)(getpid() + time(NULL)));
	t = rand() % 10;
	return t;
}


//p ����
void p(int sem_id, int sem_num) {
	struct sembuf sbf;
	sbf.sem_num = sem_num;
	sbf.sem_flg = 0;
	sbf.sem_op = -1;
	semop(sem_id, &sbf, 1);
}


//v����
void v(int sem_id, int sem_num) {
	struct sembuf sbf;
	sbf.sem_num = sem_num;
	sbf.sem_flg = 0;
	sbf.sem_op = 1;
	semop(sem_id, &sbf, 1);
}


int main(int argc, char* argv[]) {
	//�������id
	pid_t pid_p;
	pid_t pid_c;
	time_t nowtime;
	//�����ź�����,�����źű�ʶ��
	int sem_id = semget(SEM_KEY, 3, IPC_CREAT | 0660);

	if (sem_id == -1) {
		printf("�ź���������ʧ��\n");
		_Exit(1);
	}
	//�Զ���union semun�ṹ
	union  semun
	{
		int value;
		struct semd_ds* buf;
		unsigned short int* array;
		struct seminfo* _buf;
	};

	//��ʼ���ź�����
	union semun sem_val;
	sem_val.value = BUFFER_SIZE;
	//empty��ʼ��Ϊ����������
	semtcl(sem_id, SEM_EMPTY, SETVAL, sem_val);
	sem_val.value = 0;
	//��ʼ��fullΪ0
	semtcl(sem_id, SEM_FULL, SETVAL, sem_val);
	sem_val.value = 1;
	//��ʼ��mutexΪ1
	semtcl(sem_id, SEM_MUTEX, SETVAL, sem_val);

	//���������ڴ���
	int shm_id = shmget(SHM_KEY, sizeof(struct mybuffer), 0600 | IPC_CREAT);
	if (shm_id < 0) {
		//����ʧ��
		printf("���������ڴ���ʧ��\n");
		exit(1);
	}
	//��ʼ��������,�����Ըù������ķ��ʣ����������ӵ���ǰ���̵ĵ�ַ�ռ�
	struct mybuffer* shmaddr = shmat(shm_id, 0, 0);
	if (shmaddr == (void*)-1) {
		printf("�����ڴ�������ʧ��\n");
		_Exit(1);
	}
	shmaddr->head = 0;
	shmaddr->tail = 0;
	shmaddr->isEmpty = 1;
	int data;
	int index;
	int length;
	//�����߽���
	int produce_num = 0;
	while ((produce_num++) < PRODUCER_NUM) {
		if ((pid_p = fork()) < 0) {
			printf("�����߽��̴���ʧ��\n");
			exit(1);
		}
		if (pid_p == 0) {
			//�������ڴ������ӵ������߽��̵ĵ�ַ�ռ���
			if ((shmaddr =static_cast<struct mybuffer*>( shmat(shm_id, 0, 0))) == (void*)-1) {
				printf("�����ڴ�����ʧ��\n");
				exit(1);
			}
			//д������
			for (int i = 0; i < PRODUCER_TIMES; i++) {
				p(sem_id, SEM_EMPTY);//p(empty)
				int data = getRandom();
				sleep(data);
				p(sem_id, SEM_MUTEX);//p(mutex)
				shmaddr->buffer[shmaddr->tail] = data;
				shmaddr->isEmpty = 0;
				shmaddr->tail = (shmaddr->tail + 1) % BUFFER_SIZE;
				nowtime = time(NULL);
				printf("%02dʱ%02d��%02d��\n", localtime(&nowtime)->tm_hour, localtime(&nowtime)->tm_min, localtime(&nowtime)->tm_sec);
				printf("������%d ������%dд�뵽��������\n",produce_num,data);
				printf("��ǰ�����������ݣ�");
				//���㵱ǰ���������ݵĳ���
				int buffer_length = (shmaddr->tail + BUFFER_SIZE - shmaddr->head) % BUFFER_SIZE;
				if (buffer_length == 0) buffer_length = 3;
				for (int i = 0; i < buffer_length; i++) {
					printf("%d\t", shmaddr->buffer[(shmaddr->head + i)%BUFFER_SIZE]);
				}
				printf("\n\n");
				fflush(stdout);//ˢ�»�����
				v(shm_id, SEM_FULL);//v(full)
				v(shm_id, SEM_MUTEX);//v(mutex)

			}
			//�Ͽ������ڴ�����
			shmdt(shmaddr);
			exit(0);
		}

	}
	int cus_num = 0;
	while (cus_num++ < CUSTOMER_NUM) {
		if ((pid_c = fork()) < 0) {
			printf("�����߽��̴���ʧ��\n");
			exit(1);
		}
		//���������߽���
		if (pid_c == 0) {
			if ((shmaddr =static_cast<struct mybuffer*> (shmat(shm_id, 0, 0)) )== (void*)-1) {
				printf("�����ڴ�����ʧ��\n");
				exit(1);
			}
			for (int i = 0; i < CUSTOMER_TIMES; i++) {//�����߶�ȡ����
				p(shm_id, SEM_FULL);
				int sleeptime = getRandom();
				sleep(sleeptime);
				p(shm_id, SEM_MUTEX);
				data = shmaddr->buffer[shmaddr->head];
				shmaddr->head = (shmaddr->head + 1) % BUFFER_SIZE;
				shmaddr->isEmpty = (shmaddr->head == shmaddr->tail);
				nowtime = time(NULL);
				printf("��ǰʱ�䣺%02dʱ%02d��%02d��\n", localtime(&nowtime)->tm_hour, localtime(&nowtime)->tm_min, localtime(&nowtime)->tm_sec);
				printf("������%d ������%d�ӻ�����ȡ��.\n", num_c, data);
				int bufflength = (shmaddr->tail + BUFFER_SIZE - shmaddr->head) % BUFFER_SIZE;
				if (bufflength == 0)
					printf("��ǰ������Ϊ�գ�\n\n");
				else
				{
					printf("��ǰ�������ڵ����ݣ�\n");
					for (int j = 0; j < bufflength; j++)
					{
						int index = (shmaddr->head + j) % BUFFER_SIZE;
						printf("%d ", shmaddr->buffer[index]);
					}
					printf("\n\n");
				}
				fflush(stdout);
				v(sem_id, SEM_MUTEX);    //����
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
	printf("ģ�������\n");
	fflush(stdout);
	exit(0);
}