#include<stdio.h>
#include<stdlib.h>
#include<Windows.h>
#include<uchar.h>
#include<time.h>

#define BUFFER_LEGHTH 3
#define PRODUCER_TIME 6
#define PRODUCER_NUMBER 2
#define CUSTOMER_TIME 4
#define CUSTOMER_NUMBER 3

//���建��ṹ
struct buffer {
	int buffer[BUFFER_LEGHTH];
	int head;
	int tail;
	boolean isEmpty;
};

//���干���ڴ�
struct sharedMemery {
	struct buffer data;
	HANDLE sharedMemery_empty;
	HANDLE sharedMemery_full;
	HANDLE sharedMemery_mutex;
};

//�ļ�ӳ�������
static HANDLE hMapping;

//�ļ�ӳ�����������
static HANDLE hs[PRODUCER_NUMBER + CUSTOMER_NUMBER + 1];

//��������ݣ�����ΪС��1000�������
int getRandom() {
	int num;
	srand((unsigned)(GetCurrentProcessId() + time(NULL)));
	num = rand() % 1000;
	return num;
}


//���̿�¡
void processClone(int processID) {
	char szFilename[MAX_PATH];
	char szCmdLine[MAX_PATH];
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	GetModuleFileName(NULL, szFilename, MAX_PATH);
	sprintf(szCmdLine, "\"%s\" %d", szFilename, processID);
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	//�����ӽ���
	BOOL flag = CreateProcess(szFilename, szCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	hs[processID] = pi.hProcess;
}

//���������ļ���
HANDLE buildSharedFile() {
	//�����ļ�ӳ�����
	HANDLE hMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL,  PAGE_READWRITE,0,sizeof(struct sharedMemery), "Mybuffer");
	if (hMapping != INVALID_HANDLE_VALUE) {
		//���ļ��ϴ�����ͼ,�������̾Ϳ������������һ�������ļ�
		LPVOID pData = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (pData != NULL) {//����ͼ���г�ʼ��
			ZeroMemory(pData, sizeof(struct sharedMemery));
		}
		//�ر��ļ���ͼ
		UnmapViewOfFile(pData);
	}
	return hMapping;
}


//������
int main(int argc, char* argv[]) {
	int nclone = 0;	//�����̵Ľ��̺�
	int nextpro = 1;		//��һ�����̵����к�
	SYSTEMTIME nowtime;		//��������ʱ�̵�ϵͳʱ��
	if (argc > 1) {//����в�����˵�����ӽ���
		nclone = atoi(argv[1]);	
	}
	if (nclone == 0) {//������
		printf("�����̿�ʼ����\n");
		//�������������ļ�
		hMapping = buildSharedFile();
		//����ӳ����ͼ
		HANDLE hFileMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "Mybuffer");
		LPVOID pFile = MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);

		if (pFile == NULL) {//�����ؿ�ָ��˵��������ͼʧ��
			printf("OpenFileMapping failed!\n");
		}
		//�Ի��������г�ʼ��
		struct sharedMemery* shm = (struct sharedMemery*)pFile;
		shm->data.head = 0;
		shm->data.tail = 0;
		shm->data.isEmpty = TRUE;
		//�����ź������ҳ�ʼ��
		shm->sharedMemery_mutex = CreateSemaphore(NULL, 1, 1, (LPCSTR)"SEM_MUTEX");
		shm->sharedMemery_full = CreateSemaphore(NULL, 0, BUFFER_LEGHTH, (LPCSTR)"SEM_FULL");
		shm->sharedMemery_empty = CreateSemaphore(NULL, BUFFER_LEGHTH, BUFFER_LEGHTH, (LPCSTR)"SEM_EMPTY");
		//�ر���ͼ
		UnmapViewOfFile(pFile);
		pFile = NULL;
		CloseHandle(hFileMapping);

		//�����ӽ���
		while (nextpro <= 5) {
			processClone(nextpro++);

		}
		//�ȴ��ӽ�������
		for (int i = 1; i <= 5; i++) {
			WaitForSingleObject(hs[i], INFINITE);
			CloseHandle(hs[i]);
		}
		CloseHandle(hMapping);
		hMapping = INVALID_HANDLE_VALUE;
		printf("���������н���\n");
	}
	else if (nclone >= 1 && nclone <= PRODUCER_NUMBER) {//�����߽���
		//��ͼӳ��
		HANDLE hFileMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "Mybuffer");
		LPVOID pFile = MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (pFile != NULL) {
			//��ʼ���ź���
			struct sharedMemery* shm = (struct sharedMemery*)pFile;
			HANDLE mutex= OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, (LPCSTR)"SEM_MUTEX");
			HANDLE full=OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, (LPCSTR)"SEM_FULL");
			HANDLE empty = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, (LPCSTR)"SEM_EMPTY");
			for (int i = 0; i < PRODUCER_TIME; i++) {
				//��ȡ��������ȴ�ʱ��
				int sleeptime = getRandom();
				Sleep(sleeptime);
				WaitForSingleObject(empty, INFINITE);//p(empty)
				WaitForSingleObject(mutex, INFINITE);//p(mutex)
				int data = getRandom();//����д�������
				//ѭ����������ʵ��
				shm->data.buffer[shm->data.tail] = data;
				shm->data.isEmpty = FALSE;
				shm->data.tail += 1;
				shm->data.tail %= 3;

				GetLocalTime(&nowtime);
				printf("��ǰϵͳʱ�䣺%02dʱ��%02d�֣�%02d��\n", nowtime.wHour,nowtime.wMinute,nowtime.wSecond);
				printf("�����߽���%d��%dд�뵽��������\n", nclone, data);
				printf("��ǰ�������ڵ����ݣ�\n");
				int bufferlengh = (shm->data.tail + 3 - shm->data.head) % 3;		//���㵱ǰ���������ݵ�����
				if (bufferlengh == 0) bufferlengh = 3;
				//�����ǰ������������
				for (int i = 0; i < bufferlengh; i++) {
					printf("%d\t", shm->data.buffer[(shm->data.head + i) % 3]);
				}
				printf("\n\n");
				ReleaseSemaphore(full, 1, NULL);//v(full)
				ReleaseSemaphore(mutex, 1, NULL);//v(mutex);

			}
			UnmapViewOfFile(pFile);	//�ر���ͼ
			pFile = NULL;	

		}
		else {
			printf("OpenFileMapping failed.\n");
		}
		CloseHandle(hFileMapping);
	}
	else if (nclone > PRODUCER_NUMBER && nclone <= PRODUCER_NUMBER + CUSTOMER_NUMBER) {//�����߽���
		//ӳ����ͼ
		HANDLE hFileMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "Mybuffer");
		LPVOID pFile = MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (pFile == NULL) {
			printf("OpenFileMapping failed.\n");
		}
		else {
			//�����ź�������ʼ��
			struct sharedMemery* shm = (struct sharedMemery*)(pFile);
			HANDLE empty = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, (LPCSTR)"SEM_EMPTY");
			HANDLE full = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, (LPCSTR)"SEM_FULL");
			HANDLE mutex = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, (LPCSTR)"SEM_MUTEX");
			for (int i = 0; i < CUSTOMER_TIME; i++) {
				int sleeptime = getRandom();
				Sleep(sleeptime);
				WaitForSingleObject(full, INFINITE);//P(full);
				WaitForSingleObject(mutex, INFINITE);//p(mutex)
				int data = shm->data.buffer[shm->data.head];
				shm->data.head += 1;
				shm->data.head %= 3;
				if (shm->data.head == shm->data.tail)
					shm->data.isEmpty = TRUE;
				GetSystemTime(&nowtime);
				printf("��ǰϵͳʱ�䣺%02dʱ��%02d�֣�%02d��\n", nowtime.wHour, nowtime.wMinute, nowtime.wSecond);
				printf("�����߽���%d��%d�ӻ�������ȡ��\n", nclone-2, data);
				
				int bufferlengh = (shm->data.tail + 3 - shm->data.head) % 3;		//���㵱ǰ���������ݵ�����
				if (bufferlengh == 0)
					printf("��ǰ�������Ѿ�û������\n");
				else {
					//�����ǰ������������
					printf("��ǰ�������ڵ����ݣ�\n");
									for (int i = 0; i < bufferlengh; i++) {
										printf("%d\t", shm->data.buffer[(shm->data.head + i) % 3]);
									}
				}
				printf("\n\n");
				ReleaseSemaphore(empty, 1, NULL);//v(full)
				ReleaseSemaphore(mutex, 1, NULL);//v(mutex);
			}
			UnmapViewOfFile(pFile);
			pFile = NULL;

		}
	}
	else;
	return 0;
}