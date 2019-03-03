#include <sys/types.h> 
#include <stdio.h> 
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>

void executeCommand(){
    char *command = "ps";
    char *args[3];
    args[0] = "ps";
    args[1] = "f";
    args[2] = NULL;
    execvp(command,args);//execvp command is a type of exec commands.
}
int main(){

int pid = fork();
wait(NULL);
if(pid > 0){
    printf("Child finished execution.\n");
}else if(pid == 0){
    executeCommand();
}
}