#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define READ_END 0
#define WRITE_END 1

void check(int fd[]);
void create_child(int fd[]);

int main(int argc, char * argv[]){
    int fd[2];
    int pid = 0;

    if(pipe(fd)<0){
        exit(1);
    }
    
    pid = fork();
    if(pid > 0){
        close(fd[READ_END]);
        int i=2;
        for(;i<=35;++i){
            write(fd[WRITE_END], &i, sizeof i);
        }
        close(fd[WRITE_END]);
        wait((int*)0);
        exit(0);
    }
    else if(pid == 0){
        close(fd[WRITE_END]);
        check(fd);
    }
    else{
        fprintf(2, "fork error ...\n");
        exit(1);
    }
    exit(0);
}


void check(int fd[]){
    int prime = 0;
    int num = 0;
    int read_result;

    read_result = read(fd[READ_END], &prime, sizeof prime);
    if(read_result > 0){
        printf("prime %d\n", prime);
    }
    else if(read_result == 0){
        exit(0);
    }

    int fd_new[2];
    if(pipe(fd_new)<0){
        fprintf(2, "pipe error ...\n");
        exit(1);
    }

    int pid = fork();
    if(pid < 0){
        fprintf(2, "fork error ...\n");
    }
    else if(pid > 0){
        close(fd_new[READ_END]);
        while(read(fd[READ_END], &num, sizeof num) > 0){
            if(num % prime != 0){
                write(fd_new[WRITE_END], &num, sizeof num);
            }
        }

        close(fd_new[WRITE_END]);
        wait((int*)0);
        exit(0);
    }
    
    else{
        close(fd_new[WRITE_END]);
        check(fd_new);
    }
}
