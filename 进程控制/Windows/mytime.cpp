#include <stdio.h>
#include <iostream>
#include <Windows.h>
#include<tchar.h>
#include<stdlib.h>
using namespace std;
int main(int argc, char** argv)
{
    SYSTEMTIME start, end;
    STARTUPINFO si;
    memset(&si, 0, sizeof(si));//��ʼ��
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;			//��ʼ��lpProcessInformation����
  //  TCHAR szFilename[MAX_PATH];
    //sprintf((char*)szFilename, "%s", argv[1]);	//��ʼ��lpApplicationName����
    TCHAR szCmdLine[MAX_PATH];
    swprintf(szCmdLine, L"\"%S\"  %S", argv[1], argv[2]);//��ʼ��lpCommandLine����
    //��������
    if (!CreateProcess(NULL, szCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        cout << "Create Failed!" << endl;
        exit(1);
    }
    GetSystemTime(&start);
    cout << "Create Succeed!" << endl;
    WaitForSingleObject(pi.hProcess, INFINITE);
    GetSystemTime(&end);
    int msec = end.wMilliseconds - start.wMilliseconds;
    int sec = end.wSecond - start.wSecond;
    int minute = end.wMinute - start.wMinute;
    int hour = end.wHour - start.wHour;
    //�Ż����
    while (minute < 0) {
        minute += 60;
        hour--;
    }
    while (sec < 0) {
        sec += 60;
        minute--;
    }
    while (msec < 0) {
        msec += 1000;
        sec--;
    }
    cout << "���г���:" << argv[1] << endl;
    cout << "Using Time: " << hour << " ʱ " << minute << " �� " << sec << " �� " << msec << " ���� " << endl;
    return 0;
}