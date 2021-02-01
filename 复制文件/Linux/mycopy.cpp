#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<dirent.h>
#include<utime.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/time.h>
#define buffer_size 2048//������ֽ���
#define MAX_PATH 260//���·������
void CopyFile(char* fileSource, char* fileTarget) {
    //��ԭ�ļ��������ļ�������
    int fd1 = open(fileSource, O_RDONLY);
    if (fd1 == -1) {
        printf("��Դ�ļ�ʧ��!\n");
        exit(0);
    }
    int fd2;
    struct stat statbuffer;
    struct utimbuf timeee;
    char buffer[buffer_size];
    int word_bit;
    //��ԭ�ļ�����Ϣ����stabuffer��
    stat(fileSource, &statbuffer);
    //�������ļ��������ļ�������
    fd2 = creat(filetarget, statbuffer.st_mode);
    /*
    int creat(const char *pathname, mode_t mode);
        const char* pathname    ���������ļ��������԰���·����
        mode_t mode             ��������
    return int                  �������ļ�������
    */
    if (fd2 == -1) {
        printf("�޷�����Ŀ���ļ���\n");
        exit(0);
    }
    while ((word_bit = read(fd1, buffer, buffer_size)) > 0) {
        /*
        ssize_t read(int fd, void *buf, size_t count);
            int fd ����ȡ���ļ���������
            void* buf ��ȡ�����ݴ�Ż�����������������void* ��buf�ڴ�����ʱ��Ӧ����```char*``````
?            size_t count Ҫ��ȡ�����ݵĴ�С���������fd```�е���Ŀ�����ȡ��Ӧ��С����
        return ssize_t ����ʵ�ʶ�ȡ�����ݵ���Ŀ��ʧ�ܷ���-1
        */
        if (write(fd2, buffer, word_bit) != word_bit) {//д��Ŀ���ļ�
            /*
            ssize_t write(int fd, const void *buf, size_t count);
                int fd Ҫд����ļ���������
                const void *buf Ҫд������ݡ�
                size_t count Ҫд������ݴ�С
            return ssize_t �ɹ�����ʵ��д�����Ŀ��ʧ�ܷ���-1
            */
            printf("д���ļ�����!\n");
            exit(0);
        }
    }
    timeee.actime = statbuffer.st_atime;//�޸�ʱ������
    timeee.modtime = statbuffer.st_mtime;
    //�޸��ļ��ķ���ʱ����޸�ʱ��
    utime(fileTarget, &timeee);
    /*
    int utime( const char *pathname, const struct utimbuf *times );
        ����ֵ�����ɹ��򷵻�0���������򷵻�-1
    */
    close(fd1);
    close(fd2);
}
void myCopyAllFile(char* fileSource, char* fileTarget) {
    char source[MAX_PATH];
    char target[MAX_PATH];
    struct stat statbuffer;
    struct utimbuf timeee;
    struct dirent* entry;
    /*
    struct dirent
    2 {
    3    long d_ino; /* inode number �����ڵ��
    4    off_t d_off; /* offset to this dirent ��Ŀ¼�ļ��е�ƫ��
    5    unsigned short d_reclen; /* length of this d_name �ļ�����
    6    unsigned char d_type; /* the type of d_name �ļ�����
    7    char d_name[NAME_MAX + 1]; /* file name (null-terminated) �ļ������255�ַ�
    8
}   */
    DIR* dir;
    /*
    struct __dirstream
    {
    void *__fd; /* `struct hurd_fd' pointer for descriptor.
    char* __data; /* Directory block.
    int __entry_data; /* Entry number `__data' corresponds to.
    char* __ptr; /* Current pointer into the block.
    int __entry_ptr; /* Entry number `__ptr' corresponds to.
    size_t __allocation; /* Space allocated for the block.
    size_t __size; /* Total valid data in the block.
    __libc_lock_define(, __lock) /* Mutex lock for this structure.
};

    typedef struct __dirstream DIR;
*/
    strcpy(source, fileSource);
    strcpy(target, fileTarget);
    dir = opendir(source);//��Ŀ¼������ָ��DIR�ṹ��ָ��
    /*
    DIR *opendir(const char *name);
        const char *name ���򿪵�Ŀ¼·��
    return DIR ����Ŀ¼���ݽṹ
    */
    if (opendir(target) == NULL) {//Ŀ��·���ļ���Ϊ��
        stat(source, &statbuffer);
        /*
        int lstat(const char *pathname, struct stat *statbuf);
            const char *pathname ��Ҫ�жϵ��ļ���·��
            struct stat *statbuf ���ڱ����ļ�����
        return int 0�ɹ���-1ʧ��
        */
        mkdir(target, statbuffer.st_mode);
    }
    while ((entry = readdir(dir)) != NULL) {//��Ŀ¼
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {//���ļ��к�Ŀ¼
            continue;
        }
        strcpy(source, fileSource);
        strcpy(target, fileTarget);
        strcat(source, "/");
        strcat(source, entry->d_name);
        strcat(target, "/");
        strcat(target, entry->d_name);
        /*
        * ��Ҫע����ǣ���Ҫ���ж��ǲ��Ƿ��������ļ����Է��������ļ�����S_ISREG�����ж�ʱ����ֽ����ж�Ϊ��ͨ�ļ��������
        * �������������жϵ��������ļ���ָ����ļ������͡������Ҫ���ж��ǲ��������ļ���
        */
        if (entry->d_type == 10) {//����Ƿ�������
            stat(source, &statbuffer);
            char buffer[buffer_size];
            int len = readlink(source, buffer, MAX_PATH);//����ֵ��������ָ��·���ĳ���
            /*
            ssize_t readlink(const char *pathname, char *buf, size_t bufsiz);
                const cha* pathname     ����ȡ�����ӵ�·��
                char* buf               �������е����ݱ��浽buf��
                size_t bufsiz           buf�������Ĵ�С
            return ssize_t              ����ֵ��������ָ��·���ĳ���
            */
            if (symlink(buffer, target) == -1) {
                /*
                int symlink(const char *target, const char *linkpath);
                    const char* target         �������������ӵ�·��
                    const char* linkpath       ����������������ָ���·��
                    return int                 �ɹ�����0�����򷵻�-1
                */
                printf("������ʧ�ܣ�\n");
            }
            struct stat temp_statbuff;
            lstat(source, &temp_statbuff);
            /*
             int lstat(const char *pathname, struct stat *statbuf);
                const char *pathname ��Ҫ�жϵ��ļ���·��
                struct stat *statbuf ���ڱ����ļ�����
            return int 0�ɹ���-1ʧ��
             */
            struct timeval times[2];//һ��������ʱ�䣬һ���޸��ļ�ʱ��
            times[0].tv_sec = temp_statbuff.st_atime;
            times[0].tv_usec = 0;
            times[1].tv_sec = temp_statbuff.st_mtime;
            times[1].tv_usec = 0;
            lutimes(target, times);
      
        }
        else if (entry->d_type == 4) {//�����Ŀ¼
            myCopyAllFile(source, target);
        }
        else {//������ļ�
            CopyFile(source, target);//�����ļ����ƺ���
        }
    }
    //����ٴ��޸ĸ���ʱ������
    strcpy(source, fileSource);
    strcpy(target, fileTarget);
    stat(source, &statbuffer);
    timeee.actime = statbuffer.st_atime;
    timeee.modtime = statbuffer.st_mtime;
    //�޸��ļ���  ����ʱ�� �� �޸�ʱ��
    utime(target, &timeee);
    //һ���ļ��ķ��ʺ��޸�ʱ�������utime����
    //��target���ļ�ʱ������ �� source����ļ�һ��
}
int main(int argc, char* argv[]) {
    struct stat statbuffer;
    struct utmbuf timeee;
    DIR* dir;
    if (argc != 3) {
        printf("��������\n");
        exit(0);
    }
    else {
        if ((dir = opendir(argv[1])) == NULL) {
            printf("Դ�ļ��򿪴���!\n");
            exit(0);
        }
        if ((dir = opendir(argv[2])) == NULL) {//Ŀ��·��
            stat(argv[1], &statbuffer);//��Դ�ļ��е��ļ�������Ϣ����stabuffer��
            mkdir(argv[2], statbuffer.st_mode);//����Ŀ¼
            timeee.actime = statbuffer.st_atime;//�ļ����ݵ�����ȡʱ��
            timeee.modtime = statbuffer.st_mtime;//�ļ����ݵ�����޸�ʱ��
            utime(argv[2], &timeee);//��utime���������޸�Ŀ���ļ��ķ���ʱ����޸�ʱ��
        }
        myCopyAllFile(argv[1], argv[2]);//��ʼ����
        printf("�������\n");
    }
    return 0;
}

