
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wait.h>


const int SIZE = 4096;

int main( int argc, char** argv)
{
	char *execNameOfConsumerProducer = argv[1];
	int numberOfProcesses = atoi(argv[2]);
	char *message = argv[3];
	
	int shm_fd;
	void *ptr;
	const char *name = "OS"; 

	int count = 0;
	if(numberOfProcesses< 2){
		printf("You can not give a number less than 2 for number of processes");
		return EXIT_SUCCESS;
	}
	printf("The executable file name is  : %s\n",execNameOfConsumerProducer);
	printf("Number of processes is : %d\n", numberOfProcesses);
    printf("The sentence to be chinesse whispered is : %s\n",message);
	//==========================write the shared memory==============================
	// shared memory space is being created
	shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
	//  the size of the shared memory segment is configured
	ftruncate(shm_fd,SIZE);
	/* now map the shared memory segment in the address space of the process */
	ptr = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (ptr == MAP_FAILED) {
		printf("Map failed\n");
		return -1;
	}
	// Now write to the shared memory region.
	sprintf(ptr,"%s",message);
	//Increment the value of ptr accordingly after each write
	ptr += strlen(message);
	

	//==========================write the shared memory============================== 


	printf("Parent: Playing Chinese whisper with %d processes.\n",numberOfProcesses);
	int pid = 0;
    for(int i = 0; i < numberOfProcesses; i++)
	{
		pid = fork();
		wait(NULL);
		if (pid < 0){
                printf("Fork failed");
                return 1;}
		if (pid == 0){
				char snum[5];
				sprintf(snum, "%d", i+1);
				char sNumProcess[5];
				sprintf(sNumProcess, "%d", numberOfProcesses);
				char *command = execNameOfConsumerProducer;
    			char *args[4];
				args[0] = execNameOfConsumerProducer;
				args[1] = snum;
				args[2] = sNumProcess;
				args[3] = NULL;
				execvp(command,args);//execvp command is a type of exec commands.
				}
	}
	printf("Parent terminating...\n");
	
	return 0;
}