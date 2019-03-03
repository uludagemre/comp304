#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <wait.h>
#include <time.h>
#include <unistd.h>

#define MAX_STRING_LENGTH 100

void strreplace(char *s, int firstIndex, int secondIndex)
{

     char temp;
     temp = s[firstIndex];
     s[firstIndex] = s[secondIndex];
     s[secondIndex] = temp;

}
void randomlyChangeTheMessage(char *message)
{
    int messageLength = strlen(message)-1;
    srand(time(NULL));
    int firstChangeableIndex = rand() % messageLength;
    int secondChangeableIndex = rand() % messageLength;
    strreplace(message,firstChangeableIndex,secondChangeableIndex);
    
}


int main(int argc, char **argv)
{

    const char *name = "OS";
    const int SIZE = 4096;
    int count = atoi(argv[1]);
    int finalCount = atoi(argv[2]);
    int shm_fd;
    char *ptr;
    char *messageToBEChanged;
    // char *messageToBEChanged2;
    

    /* open the shared memory segment */
    shm_fd = shm_open(name, O_RDONLY, 0666);
    if (shm_fd == -1)
    {
        printf("shared memory failed\n");
        exit(-1);
    }

    /* now map the shared memory segment in the address space of the process */
    ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED)
    {
        printf("Map failed\n");
        exit(-1);
    }

    /* now read from the shared memory region */
    printf("Child %d: ", count);
    printf("%s\n",ptr);

    
    int length = strlen(ptr);
    char finalArray[length];
    for(int i = 0; i < length; i++)
    {
        finalArray[i] = *(ptr+i);
    }
     finalArray[length] = '\0';
    // char finalArray[20];
    sleep(1.2712); //This line of code is to make randomize operation more clear
    randomlyChangeTheMessage(finalArray);

    shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
	//  the size of the shared memory segment is configured
	ftruncate(shm_fd,SIZE);
	/* now map the shared memory segment in the address space of the process */
	messageToBEChanged = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (messageToBEChanged == MAP_FAILED) {
		printf("Map failed\n");
		return -1;
	}

    sprintf(messageToBEChanged,"%s",finalArray);
    messageToBEChanged+=strlen(finalArray);

    /* remove the shared memory segment if the process is over*/
    if (count == finalCount) {
        printf("Removing the shared memory segment!\n");
        if (shm_unlink(name) == -1) {
    	printf("Error removing %s\n",name);
    	exit(-1);
    } 
    }
    
    
    return 0;
}