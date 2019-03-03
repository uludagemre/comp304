#include <sys/types.h> 
#include <stdio.h> 
#include <unistd.h>
#include <wait.h>



int main(){
    
    int pid1 = fork();
    
    wait(NULL);
    int pid2 = fork();
    
    wait(NULL);
    int pid3 = fork();
    
    wait(NULL);
    
    int pid4 = fork();
    
    wait(NULL);

    int levelZeroCondition = (pid1 != 0 & pid2 != 0 & pid3 != 0 & pid4 != 0);
    int levelOneCondition = (pid1 == 0 & pid2 != 0 & pid3 != 0 & pid4 != 0)|(pid1 != 0 & pid2 == 0 & pid3 != 0 & pid4 != 0)|(pid1 == 0 & pid2 != 0 & pid3 == 0 & pid4 != 0)|(pid1 != 0 & pid2 != 0 & pid3 != 0 & pid4 == 0);
    int levelTwoCondition = (pid1 == 0 & pid2 == 0 & pid3 != 0 & pid4 != 0)|(pid1 != 0 & pid2 == 0 & pid3 == 0 & pid4 != 0)|(pid1 != 0 & pid2 != 0 & pid3 == 0 & pid4 == 0)|(pid1 == 0 & pid2 != 0 & pid3 != 0 & pid4 == 0)|(pid1 == 0 & pid2 != 0 & pid3 == 0 & pid4 != 0)|(pid1 != 0 & pid2 == 0 & pid3 != 0 & pid4 == 0);
    int levelThreeCOndition = (pid1 == 0 & pid2 == 0 & pid3 == 0 & pid4 != 0)|(pid1 == 0 & pid2 == 0 & pid3 != 0 & pid4 == 0)|(pid1 == 0 & pid2 != 0 & pid3 == 0 & pid4 == 0)|(pid1 != 0 & pid2 == 0 & pid3 == 0 & pid4 == 0);
    int levelFourCondition = (pid1 == 0 & pid2 == 0 & pid3 == 0 & pid4 == 0);
    if(levelZeroCondition){
        
        printf("Base Process ID: %d, level: 0\n",getpid());
    }else if(levelOneCondition) {
        
        printf("Process ID: %d,Parent ID: %d level: 1\n",getpid(),getppid());
    }else if(levelTwoCondition) {
        
        printf("Process ID: %d,Parent ID: %d level: 2\n",getpid(),getppid());
    }else if(levelThreeCOndition) {
        
        printf("Process ID: %d,Parent ID: %d level: 3\n",getpid(),getppid());
    }else if(levelFourCondition) {
        
        printf("Process ID: %d,Parent ID: %d level: 4\n",getpid(),getppid());
    }
    
}