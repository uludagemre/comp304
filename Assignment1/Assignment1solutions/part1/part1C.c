#include <sys/types.h> 
#include <stdio.h> 
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>


int main(){
    
    int pid = fork();
    
    if (pid > 0){
        sleep(5);
        printf("\n");
        printf("In parent process:\n");
        printf("The parent process id : %d\n",getpid());
       
    }
    
    else if(pid == 0){
        printf("\n");
        printf("In zombie process:\n");
        printf("The zombie process pid is: %d\n",getpid());
        printf("The parent process id : %d\n",getppid());
        exit(0);
    }

    return 0;

}