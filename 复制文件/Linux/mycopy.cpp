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
#define buffer_size 2048//最大复制字节数
#define MAX_PATH 260//最大路径长度
void CopyFile(char* fileSource, char* fileTarget) {
    //打开原文件，返回文件描述符
    int fd1 = open(fileSource, O_RDONLY);
    if (fd1 == -1) {
        printf("打开源文件失败!\n");
        exit(0);
    }
    int fd2;
    struct stat statbuffer;
    struct utimbuf timeee;
    char buffer[buffer_size];
    int word_bit;
    //将原文件的信息放入stabuffer中
    stat(fileSource, &statbuffer);
    //创建新文件，返回文件描述符
    fd2 = creat(filetarget, statbuffer.st_mode);
    /*
    int creat(const char *pathname, mode_t mode);
        const char* pathname    待创建的文件名（可以包含路径）
        mode_t mode             创建属性
    return int                  待创建文件描述符
    */
    if (fd2 == -1) {
        printf("无法创建目标文件！\n");
        exit(0);
    }
    while ((word_bit = read(fd1, buffer, buffer_size)) > 0) {
        /*
        ssize_t read(int fd, void *buf, size_t count);
            int fd 待读取的文件的描述符
            void* buf 读取的内容存放缓冲区。尽管这里是void* ，buf在创建的时候应该是```char*``````
?            size_t count 要读取的内容的大小。如果大于fd```中的数目，则读取相应大小内容
        return ssize_t 返回实际读取的内容的数目。失败返回-1
        */
        if (write(fd2, buffer, word_bit) != word_bit) {//写入目标文件
            /*
            ssize_t write(int fd, const void *buf, size_t count);
                int fd 要写入的文件的描述符
                const void *buf 要写入的内容。
                size_t count 要写入的内容大小
            return ssize_t 成功返回实际写入的数目。失败返回-1
            */
            printf("写入文件错误!\n");
            exit(0);
        }
    }
    timeee.actime = statbuffer.st_atime;//修改时间属性
    timeee.modtime = statbuffer.st_mtime;
    //修改文件的访问时间和修改时间
    utime(fileTarget, &timeee);
    /*
    int utime( const char *pathname, const struct utimbuf *times );
        返回值：若成功则返回0，若出错则返回-1
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
    3    long d_ino; /* inode number 索引节点号
    4    off_t d_off; /* offset to this dirent 在目录文件中的偏移
    5    unsigned short d_reclen; /* length of this d_name 文件名长
    6    unsigned char d_type; /* the type of d_name 文件类型
    7    char d_name[NAME_MAX + 1]; /* file name (null-terminated) 文件名，最长255字符
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
    dir = opendir(source);//打开目录，返回指向DIR结构的指针
    /*
    DIR *opendir(const char *name);
        const char *name 待打开的目录路径
    return DIR 返回目录数据结构
    */
    if (opendir(target) == NULL) {//目标路径文件夹为空
        stat(source, &statbuffer);
        /*
        int lstat(const char *pathname, struct stat *statbuf);
            const char *pathname 需要判断的文件的路径
            struct stat *statbuf 用于保存文件属性
        return int 0成功，-1失败
        */
        mkdir(target, statbuffer.st_mode);
    }
    while ((entry = readdir(dir)) != NULL) {//读目录
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {//非文件夹和目录
            continue;
        }
        strcpy(source, fileSource);
        strcpy(target, fileTarget);
        strcat(source, "/");
        strcat(source, entry->d_name);
        strcat(target, "/");
        strcat(target, entry->d_name);
        /*
        * 需要注意的是，需要先判断是不是符号链接文件。对符号链接文件进行S_ISREG（）判断时会出现将其判断为普通文件的情况。
        * 可能是由于他判断的是链接文件所指向的文件的类型。因此需要先判断是不是链接文件。
        */
        if (entry->d_type == 10) {//如果是符号链接
            stat(source, &statbuffer);
            char buffer[buffer_size];
            int len = readlink(source, buffer, MAX_PATH);//返回值是软链接指向路径的长度
            /*
            ssize_t readlink(const char *pathname, char *buf, size_t bufsiz);
                const cha* pathname     待读取软链接的路径
                char* buf               软链接中的内容保存到buf中
                size_t bufsiz           buf缓冲区的大小
            return ssize_t              返回值是软链接指向路径的长度
            */
            if (symlink(buffer, target) == -1) {
                /*
                int symlink(const char *target, const char *linkpath);
                    const char* target         待创建的软链接的路径
                    const char* linkpath       待创建的软链接所指向的路径
                    return int                 成功返回0，否则返回-1
                */
                printf("软链接失败！\n");
            }
            struct stat temp_statbuff;
            lstat(source, &temp_statbuff);
            /*
             int lstat(const char *pathname, struct stat *statbuf);
                const char *pathname 需要判断的文件的路径
                struct stat *statbuf 用于保存文件属性
            return int 0成功，-1失败
             */
            struct timeval times[2];//一个最后访问时间，一个修改文件时间
            times[0].tv_sec = temp_statbuff.st_atime;
            times[0].tv_usec = 0;
            times[1].tv_sec = temp_statbuff.st_mtime;
            times[1].tv_usec = 0;
            lutimes(target, times);
      
        }
        else if (entry->d_type == 4) {//如果是目录
            myCopyAllFile(source, target);
        }
        else {//如果是文件
            CopyFile(source, target);//调用文件复制函数
        }
    }
    //最后再次修改更新时间属性
    strcpy(source, fileSource);
    strcpy(target, fileTarget);
    stat(source, &statbuffer);
    timeee.actime = statbuffer.st_atime;
    timeee.modtime = statbuffer.st_mtime;
    //修改文件的  访问时间 和 修改时间
    utime(target, &timeee);
    //一个文件的访问和修改时间可以用utime更改
    //让target的文件时间属性 和 source里的文件一样
}
int main(int argc, char* argv[]) {
    struct stat statbuffer;
    struct utmbuf timeee;
    DIR* dir;
    if (argc != 3) {
        printf("参数错误\n");
        exit(0);
    }
    else {
        if ((dir = opendir(argv[1])) == NULL) {
            printf("源文件打开错误!\n");
            exit(0);
        }
        if ((dir = opendir(argv[2])) == NULL) {//目标路径
            stat(argv[1], &statbuffer);//将源文件中的文件属性信息填入stabuffer里
            mkdir(argv[2], statbuffer.st_mode);//创建目录
            timeee.actime = statbuffer.st_atime;//文件数据的最后存取时间
            timeee.modtime = statbuffer.st_mtime;//文件数据的最后修改时间
            utime(argv[2], &timeee);//用utime（）函数修改目标文件的访问时间和修改时间
        }
        myCopyAllFile(argv[1], argv[2]);//开始复制
        printf("复制完成\n");
    }
    return 0;
}

