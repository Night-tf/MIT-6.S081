#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"



int
main(int argc, char *argv[])
{

  if (argc < 2){
    printf("Error! This function need 1 argument!!!\n");
    exit(1);
  }  
  else if (argc > 2){
    printf("Error! This function need only 1 argument!!!\n");
    exit(1);
  }  

  int n = atoi(argv[1]);
  sleep(n);


  exit(0);
}
