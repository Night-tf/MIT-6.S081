#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"

#define MSG_SIZE 512
#define MAX_LEN 64
#define STD_IN 0

// 在命令行中运行一个命令(程序)的时候，它的main()函数
// 获取到的命令行参数是从这个命令自己开始的，
// 而不是从这一行命令开头开始的

// 运行一个命令的时候，是运行的这个文件里的main()函数
// 如果创建了同名函数，那么可以供其他命令来调用
int main(int argc, char *argv[]){
    if(argc<2){
        printf("usage: xargs <command> ...\n");
        exit(0);
    }

    // 1. 获取xargs后面的命令行参数
    int xargc = argc - 1;
    char *xargv[MAXARG];
    char temp_argv[MAXARG][MAX_LEN];
    for(int i=1; i<argc; ++i){
        // xargv[i-1] = argv[i];
        strcpy(temp_argv[i-1], argv[i]);
    }

    // 2. 读取stdout中的输出
    // 将其加入xargv中(当作命令行参数，这也就是xargs原本的作用)
    char buf[MSG_SIZE];
    char *p = buf;

    int pid = 0;
    while(read(STD_IN, p, 1) == 1){
        if(*p == '\n'){
            *p = 0;
            // printf("buf: %s\n", buf);
            p = buf;

            pid = fork();
            if(pid < 0){
                fprintf(2, "fork() error ...\n");
            }
            else if(pid == 0){
                memmove(temp_argv[xargc], buf, MAX_LEN);
                // printf("xargv[xargc]: %s\n", temp_argv[xargc]);
                xargc++;
                for(int i=0; i<xargc; ++i){
                    xargv[i] = temp_argv[i];
                    // printf("xargv[xargc]: %s\n", xargv[i]);
                }
                exec(xargv[0], xargv);
                exit(0);
            }
            else{
                wait((int*) 0);
            }
        }
        else{
            ++p;
        }
    }
    exit(0);
}