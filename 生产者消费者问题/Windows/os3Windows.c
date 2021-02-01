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

//定义缓冲结构
struct buffer {
	int buffer[BUFFER_LEGHTH];
	int head;
	int tail;
	boolean isEmpty;
};

//定义共享内存
struct sharedMemery {
	struct buffer data;
	HANDLE sharedMemery_empty;
	HANDLE sharedMemery_full;
	HANDLE sharedMemery_mutex;
};

//文件映射对象句柄
static HANDLE hMapping;

//文件映射对象句柄数组
static HANDLE hs[PRODUCER_NUMBER + CUSTOMER_NUMBER + 1];

//输入的数据，数据为小于1000的随机数
int getRandom() {
	int num;
	srand((unsigned)(GetCurrentProcessId() + time(NULL)));
	num = rand() % 1000;
	return num;
}


//进程克隆
void processClone(int processID) {
	char szFilename[MAX_PATH];
	char szCmdLine[MAX_PATH];
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	GetModuleFileName(NULL, szFilename, MAX_PATH);
	sprintf(szCmdLine, "\"%s\" %d", szFilename, processID);
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	//创建子进程
	BOOL flag = CreateProcess(szFilename, szCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	hs[processID] = pi.hProcess;
}

//创建共享文件区
HANDLE buildSharedFile() {
	//创建文件映射对象
	HANDLE hMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL,  PAGE_READWRITE,0,sizeof(struct sharedMemery), "Mybuffer");
	if (hMapping != INVALID_HANDLE_VALUE) {
		//在文件上创建视图,这样进程就可以像访问主存一样访问文件
		LPVOID pData = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (pData != NULL) {//对视图进行初始化
			ZeroMemory(pData, sizeof(struct sharedMemery));
		}
		//关闭文件视图
		UnmapViewOfFile(pData);
	}
	return hMapping;
}


//主函数
int main(int argc, char* argv[]) {
	int nclone = 0;	//本进程的进程号
	int nextpro = 1;		//下一个进程的序列号
	SYSTEMTIME nowtime;		//进程运行时刻的系统时间
	if (argc > 1) {//如果有参数，说明是子进程
		nclone = atoi(argv[1]);	
	}
	if (nclone == 0) {//主进程
		printf("主进程开始运行\n");
		//创建共享数据文件
		hMapping = buildSharedFile();
		//创建映射视图
		HANDLE hFileMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "Mybuffer");
		LPVOID pFile = MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);

		if (pFile == NULL) {//若返回空指针说明创建视图失败
			printf("OpenFileMapping failed!\n");
		}
		//对缓冲区进行初始化
		struct sharedMemery* shm = (struct sharedMemery*)pFile;
		shm->data.head = 0;
		shm->data.tail = 0;
		shm->data.isEmpty = TRUE;
		//创建信号量并且初始化
		shm->sharedMemery_mutex = CreateSemaphore(NULL, 1, 1, (LPCSTR)"SEM_MUTEX");
		shm->sharedMemery_full = CreateSemaphore(NULL, 0, BUFFER_LEGHTH, (LPCSTR)"SEM_FULL");
		shm->sharedMemery_empty = CreateSemaphore(NULL, BUFFER_LEGHTH, BUFFER_LEGHTH, (LPCSTR)"SEM_EMPTY");
		//关闭视图
		UnmapViewOfFile(pFile);
		pFile = NULL;
		CloseHandle(hFileMapping);

		//创建子进程
		while (nextpro <= 5) {
			processClone(nextpro++);

		}
		//等待子进程运行
		for (int i = 1; i <= 5; i++) {
			WaitForSingleObject(hs[i], INFINITE);
			CloseHandle(hs[i]);
		}
		CloseHandle(hMapping);
		hMapping = INVALID_HANDLE_VALUE;
		printf("主进程运行结束\n");
	}
	else if (nclone >= 1 && nclone <= PRODUCER_NUMBER) {//生产者进程
		//视图映射
		HANDLE hFileMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "Mybuffer");
		LPVOID pFile = MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (pFile != NULL) {
			//初始化信号量
			struct sharedMemery* shm = (struct sharedMemery*)pFile;
			HANDLE mutex= OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, (LPCSTR)"SEM_MUTEX");
			HANDLE full=OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, (LPCSTR)"SEM_FULL");
			HANDLE empty = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, (LPCSTR)"SEM_EMPTY");
			for (int i = 0; i < PRODUCER_TIME; i++) {
				//获取进程随机等待时间
				int sleeptime = getRandom();
				Sleep(sleeptime);
				WaitForSingleObject(empty, INFINITE);//p(empty)
				WaitForSingleObject(mutex, INFINITE);//p(mutex)
				int data = getRandom();//生成写入的数据
				//循环缓冲区的实现
				shm->data.buffer[shm->data.tail] = data;
				shm->data.isEmpty = FALSE;
				shm->data.tail += 1;
				shm->data.tail %= 3;

				GetLocalTime(&nowtime);
				printf("当前系统时间：%02d时，%02d分，%02d秒\n", nowtime.wHour,nowtime.wMinute,nowtime.wSecond);
				printf("生产者进程%d把%d写入到缓冲区中\n", nclone, data);
				printf("当前缓冲区内的数据：\n");
				int bufferlengh = (shm->data.tail + 3 - shm->data.head) % 3;		//计算当前缓冲区数据的数量
				if (bufferlengh == 0) bufferlengh = 3;
				//输出当前缓冲区的数据
				for (int i = 0; i < bufferlengh; i++) {
					printf("%d\t", shm->data.buffer[(shm->data.head + i) % 3]);
				}
				printf("\n\n");
				ReleaseSemaphore(full, 1, NULL);//v(full)
				ReleaseSemaphore(mutex, 1, NULL);//v(mutex);

			}
			UnmapViewOfFile(pFile);	//关闭视图
			pFile = NULL;	

		}
		else {
			printf("OpenFileMapping failed.\n");
		}
		CloseHandle(hFileMapping);
	}
	else if (nclone > PRODUCER_NUMBER && nclone <= PRODUCER_NUMBER + CUSTOMER_NUMBER) {//消费者进程
		//映射视图
		HANDLE hFileMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "Mybuffer");
		LPVOID pFile = MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (pFile == NULL) {
			printf("OpenFileMapping failed.\n");
		}
		else {
			//创建信号量并初始化
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
				printf("当前系统时间：%02d时，%02d分，%02d秒\n", nowtime.wHour, nowtime.wMinute, nowtime.wSecond);
				printf("消费者进程%d把%d从缓冲区中取走\n", nclone-2, data);
				
				int bufferlengh = (shm->data.tail + 3 - shm->data.head) % 3;		//计算当前缓冲区数据的数量
				if (bufferlengh == 0)
					printf("当前缓冲区已经没有数据\n");
				else {
					//输出当前缓冲区的数据
					printf("当前缓冲区内的数据：\n");
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