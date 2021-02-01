#include<stdio.h>
#include<stdlib.h>
#include<Windows.h>
#include<windowsx.h>
#include<string.h>


//�����ļ��ĺ�����f1��ʾ�������ļ���f2��ʾĿ���ļ�
void cpfile(char* f1, char* f2) {
	WIN32_FIND_DATA lpFindFileData;
	//��ȡ�������ļ��������Ϣ
	HANDLE hfind = FindFirstFile(f1,&lpFindFileData);
	//�޸�Ŀ���ļ����ԣ�ȷ��ֻ���ļ��ܹ�˳������
	if ((SetFileAttributes(f1, FILE_ATTRIBUTE_NORMAL)) == 0)
	{
		printf("�޸��ļ�����ʧ��!\n");
		exit(0);
	}
	//�򿪱������ļ�
	HANDLE hf1 = CreateFile(f1, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	//����Ŀ���ļ�
	HANDLE hf2 = CreateFile(f2, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	//�޸�Ŀ���ļ�ʱ������
	SetFileTime(hf2, &lpFindFileData.ftCreationTime, &lpFindFileData.ftLastAccessTime, &lpFindFileData.ftLastWriteTime);
	//��ȡԴ�ļ��Ĵ�С
	LONG size = lpFindFileData.nFileSizeLow - lpFindFileData.nFileSizeHigh;
	//����������
	int* buffer;
	buffer = (int*)malloc(size * (sizeof(int)));
	DWORD wordbit;
	//��������
	ReadFile(hf1, buffer, size, &wordbit, NULL);
	//д������
	WriteFile(hf2, buffer, size, &wordbit, NULL);
	//�޸�Ŀ���ļ����ԣ���Դ�ļ�һ��
	if ((SetFileAttributes(f1, lpFindFileData.dwFileAttributes)) == 0)
	{
		printf("�޸��ļ�����ʧ��!\n");
		exit(0);
	}
	if ((SetFileAttributes(f2, lpFindFileData.dwFileAttributes)) == 0)
	{
		printf("�޸��ļ�����ʧ��!\n");
		exit(0);
	}
	//�رվ��
	CloseHandle(hfind);
	CloseHandle(hf1);
	CloseHandle(hf2);

}


//	�����ļ�·��
//f1��Դ�ļ���f2��Ŀ���ļ�
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
		printf("�ļ�����ʧ�ܣ�\n");
	}
	else {
		//printf("%s\n", lpfindf1data.cFileName);
		while (FindNextFile(hf1, &lpfindf1data) != 0) {
			//printf("�ļ����ҳɹ���\n");
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
			if (lpfindf1data.dwFileAttributes == 16) {//����16��ʾ��Ŀ¼
				//�ж�Ŀ¼�Ƿ�Ϊ��ǰĿ¼���߸�Ŀ¼
				if ((strcmp(lpfindf1data.cFileName, ".") != 0) && (strcmp(lpfindf1data.cFileName, "..") != 0)) {
					CreateDirectory(f2_name,NULL);
					cpdir(f1_name, f2_name);//������Ŀ¼����
					HANDLE sou = CreateFile(f1_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
					HANDLE tar = CreateFile(f2_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
					//�޸Ĵ������ļ��е�ʱ������
					SetFileTime(tar, &lpfindf1data.ftCreationTime, &lpfindf1data.ftLastAccessTime, &lpfindf1data.ftLastWriteTime);
				}
			}
			else {//����Ŀ¼ֱ�Ӹ���
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
		printf("���������������\n");
		return 0;
	}
	if (FindFirstFile(argv[1], &lpfindfiledata) == INVALID_HANDLE_VALUE)
	{
		printf("�����ļ�·��ʧ��!\n");
		exit(0);
	}
	if (FindFirstFile(argv[2], &lpfindfiledata) == INVALID_HANDLE_VALUE)
	{
		CreateDirectory(argv[2], NULL);//ΪĿ���ļ�����Ŀ¼		
	}
	//cpdir(argv[1], argv[2]);

	cpdir(argv[1], argv[2]);
	printf("���Ƴɹ�!\n");
	return 0;
}