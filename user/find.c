#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

#define MSG_SIZE 256


int compare(char *path, const char *file){
    char *p;

    for(p=path+strlen(path);p>=path && *p != '/'; --p){};
    ++p;

    return strcmp(p,file);
}


void find(char *path, const char *file){
    int fd = 0;
    struct stat st;
    struct dirent de;
    char buf[MSG_SIZE], *p;

    if((fd = open(path,0))<0){
        fprintf(2,"find: cannot open %s\n", path);
        return;
    }

    if(fstat(fd,&st)<0){
        fprintf(2,"find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch(st.type){
    case T_FILE:
        if(compare(path,file) == 0){
            printf("%s\n", path);
        }
        break;  // 在这儿break更好，因为后面可以close()来释放资源

    case T_DIR:
        while(read(fd, &de, sizeof(de)) == sizeof(de)){
            strcpy(buf, path);
            p = buf+strlen(buf);
            *p++ = '/';  // equal to *(p++), and (p++) will return p
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;

            if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0 ||
                de.inum == 0){
                continue;
            }

            // if(stat(buf, &st)<0){
            //     fprintf(2,"find: cannot stat %s\n", buf);
            //     continue;
            // }
            find(buf, file);
        }
    }

    close(fd);
    // de.inum == st.ino
    // 表示的是被创建的文件的索引（真名）
    // 比如新建一个a，上述值就是25
    // . 和 .. 的这两个文件的值都是1
    
}


// main()函数的argv是一个字符串数组
int main(int argc, char *argv[]){
    if(argc < 3){
        printf("usage: find <path_name> <file_name>\n");
        exit(0);
    }

    find(argv[1],argv[2]);
    exit(0);
}