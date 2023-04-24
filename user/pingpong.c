#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define READ_END 0
#define WRITE_END 1

int main(int argc, char *argv[]){
    int p1[2];
    int p2[2];

    if(pipe(p1)<0){
        fprintf(2, "pipe error...\n");
        exit(1);
    }
        
    if(pipe(p2)<0){
        fprintf(2, "pipe error...\n");
        exit(1);
    }

    int pid = fork();
    int n;
    char buf[1];
    
    if(pid > 0){
        close(p1[WRITE_END]);
        close(p2[READ_END]);

        write(p2[WRITE_END], " ", 1);
        n = read(p1[READ_END],buf, sizeof buf);
        if(n<0){
            fprintf(2, "read error\n");
            exit(1);
        }
        else if(n>0){
            fprintf(1, "%d: received pong\n", getpid());
        }

        close(p1[READ_END]);
        close(p2[WRITE_END]);
        exit(0);
    }
    else if(pid == 0){
        close(p1[READ_END]);
        close(p2[WRITE_END]);

        n = read(p2[READ_END],buf, sizeof buf);
        if(n<0){
            fprintf(2, "read error\n");
            exit(1);
        }
        else if(n>0){
            fprintf(1, "%d: received ping\n", getpid());
            write(p1[WRITE_END], " ", 1);
        }

        close(p1[WRITE_END]);
        close(p2[READ_END]);
        exit(0);
    }
    else{
        fprintf(2, "fork error...\n");
        exit(1);
    }

}