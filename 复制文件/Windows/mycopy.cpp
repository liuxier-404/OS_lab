#include<stdio.h>
#include<stdlib.h>
#include<Windows.h>
#include<windowsx.h>
#include<string.h>


//复制文件的函数。f1表示被复制文件，f2表示目标文件
void cpfile(char* f1, char* f2) {
	WIN32_FIND_DATA lpFindFileData;
	//获取被复制文件的相关信息
	HANDLE hfind = FindFirstFile(f1,&lpFindFileData);
	//修改目标文件属性，确保只读文件能够顺利复制
	if ((SetFileAttributes(f1, FILE_ATTRIBUTE_NORMAL)) == 0)
	{
		printf("修改文件属性失败!\n");
		exit(0);
	}
	//打开被复制文件
	HANDLE hf1 = CreateFile(f1, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	//创建目标文件
	HANDLE hf2 = CreateFile(f2, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	//修改目标文件时间属性
	SetFileTime(hf2, &lpFindFileData.ftCreationTime, &lpFindFileData.ftLastAccessTime, &lpFindFileData.ftLastWriteTime);
	//获取源文件的大小
	LONG size = lpFindFileData.nFileSizeLow - lpFindFileData.nFileSizeHigh;
	//创建缓冲区
	int* buffer;
	buffer = (int*)malloc(size * (sizeof(int)));
	DWORD wordbit;
	//读入数据
	ReadFile(hf1, buffer, size, &wordbit, NULL);
	//写入数据
	WriteFile(hf2, buffer, size, &wordbit, NULL);
	//修改目标文件属性，与源文件一致
	if ((SetFileAttributes(f1, lpFindFileData.dwFileAttributes)) == 0)
	{
		printf("修改文件属性失败!\n");
		exit(0);
	}
	if ((SetFileAttributes(f2, lpFindFileData.dwFileAttributes)) == 0)
	{
		printf("修改文件属性失败!\n");
		exit(0);
	}
	//关闭句柄
	CloseHandle(hfind);
	CloseHandle(hf1);
	CloseHandle(hf2);

}


//	复制文件路径
//f1是源文件，f2是目标文件
void cpdir(char* f1, char* f2) {
	WIN32_FIND_DATA lpfindf1data;
	WIN32_FIND_DATA lpfindf2data;
	char f1_name[4096];
	char f2_name[4096];
	lstrcpy(f1_name, f1);
	lstrcpy(f2_name, f2);
	lstrcat(f1_name, "\\*.*");
	lstrcat(f2_name, "\\*.*");

	//printf("%s\n", f1_name);
	//printf("%s\n", f2_name);
	HANDLE hf1 = FindFirstFile(f1_name, &lpfindf1data);
	HANDLE hf2 = FindFirstFile(f2_name, &lpfindf2data);

	if (hf1 == INVALID_HANDLE_VALUE || hf2 == INVALID_HANDLE_VALUE) {
		printf("文件查找失败！\n");
	}
	else {
		//printf("%s\n", lpfindf1data.cFileName);
		while (FindNextFile(hf1, &lpfindf1data) != 0) {
			//printf("文件查找成功！\n");
			memset(f1_name, '0', sizeof(f1_name));
			memset(f2_name, '0', sizeof(f2_name));
			lstrcpy(f1_name, f1);
			lstrcpy(f2_name, f2);
			lstrcat(f1_name, "\\");
			lstrcat(f2_name, "\\");
			lstrcat(f1_name, lpfindf1data.cFileName);
			lstrcat(f2_name, lpfindf1data.cFileName);
			/*printf("f1:%s\n", f1_name);
			printf("f2:%s\n", f2_name);*/
			if (lpfindf1data.dwFileAttributes == 16) {//等于16表示是目录
				//判断目录是否为当前目录或者父目录
				if ((strcmp(lpfindf1data.cFileName, ".") != 0) && (strcmp(lpfindf1data.cFileName, "..") != 0)) {
					CreateDirectory(f2_name,NULL);
					cpdir(f1_name, f2_name);//进入子目录复制
					HANDLE sou = CreateFile(f1_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
					HANDLE tar = CreateFile(f2_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
					//修改创建的文件夹的时间属性
					SetFileTime(tar, &lpfindf1data.ftCreationTime, &lpfindf1data.ftLastAccessTime, &lpfindf1data.ftLastWriteTime);
				}
			}
			else {//不是目录直接复制
				cpfile(f1_name, f2_name);
			}
		}
	}

}

int main(int argc, char* argv[]) {
	WIN32_FIND_DATA lpfindfiledata;
	/*char t1[] = { "D:\\test" };
	char t2[] = { "D:\\test1" };*/
	if (argc != 3) {
		printf("输入参数数量错误！\n");
		return 0;
	}
	if (FindFirstFile(argv[1], &lpfindfiledata) == INVALID_HANDLE_VALUE)
	{
		printf("查找文件路径失败!\n");
		exit(0);
	}
	if (FindFirstFile(argv[2], &lpfindfiledata) == INVALID_HANDLE_VALUE)
	{
		CreateDirectory(argv[2], NULL);//为目标文件创建目录		
	}
	//cpdir(argv[1], argv[2]);

	cpdir(argv[1], argv[2]);
	printf("复制成功!\n");
	return 0;
}