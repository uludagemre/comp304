

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <wait.h>
#include <stdlib.h>

#define READ_END 0
#define WRITE_END 1
#define MAX_ARRAY_SIZE 500

int maxValue(int myArray[], size_t size)
{
    size_t i;
    int maxValue = myArray[0];

    for (i = 1; i < size; ++i)
    {
        if (myArray[i] > maxValue)
        {
            maxValue = myArray[i];
        }
    }
    return maxValue;
}
void randomizeArray(int myArray[], size_t size)
{
    srand(time(NULL));
    for (size_t i = 0; i < size; i++)
    {
        int random = rand() % 100;
        myArray[i] = random;
    }
}
void distributeElementsEvenly(int myArray[], int size, int elementsRemained, int division){
    for(int i = 0; i < size; i++)
    {
        myArray[i]=division;
    }
    for(int i = 0; i < elementsRemained; i++)
    {
        myArray[i] = myArray[i]+1;
    }
}
void printArrayElements(int myArray[], int size){
 for (int i = 0; i < size; i++)
        {
            printf(" %d ", myArray[i]);
        }           
}

int main( int argc, char *argv[] )
{ 
    printf("First argument entered: %s\n", argv[1]);
    printf("Second argument entered: %s\n", argv[2]);
    
    int M = atoi(argv[1]);
    int N = atoi(argv[2]);
   
    if (N > M)
    {
        printf("Error, number of elements cannot be less than number of children processes.\n");
        return EXIT_SUCCESS;
    }
    int array[M];
    int numberOfProcesses = N;
    int totalNumberOfElements = M;
    int pipeFromPtoC[numberOfProcesses][2];
    int pipeFromCtoP[numberOfProcesses][2];

    int elementsRemained = M % N;
    int division = M / N;

    printf("The number of elements in the whole list:%d\n", M);
    printf("The number of child processes :%d\n", N);
    
    int elementAmounts[numberOfProcesses];    
    distributeElementsEvenly(elementAmounts,numberOfProcesses,elementsRemained,division);

    printf("Parent: N = %d, M = %d\n", N, M);
    int pid = getpid();
    int index = 0;
    int count = 0;
    for (int i = 0; i < numberOfProcesses; i++) // Loop will run n times to create processes and pipes.
    {
        if (pid > 0)
        {
            if (pipe(pipeFromPtoC[i]) == -1){
                fprintf(stderr, "Pipe failed");
                return 1;}
            
            if (pipe(pipeFromCtoP[i]) == -1){
                fprintf(stderr, "Pipe failed");
                return 1;}
            
            count++;
            pid = fork();
            
            if (pid < 0){
                printf("Fork failed");
                return 1;}
            if (pid == 0){
                index = i;
                sleep(count);} 
        }
    }
    //If the process is a child process do the following:
    if (pid == 0){
        int subArrayInTheProcess[MAX_ARRAY_SIZE];
        int localMax = 0;
        int subArraySize = 0;
        read(pipeFromPtoC[index][READ_END], &subArraySize, sizeof(int));
        read(pipeFromPtoC[index][READ_END], &subArrayInTheProcess, sizeof(int) * subArraySize);
        close(pipeFromPtoC[index][READ_END]);
        printf("Elements in the sub-process: ");
        printArrayElements(subArrayInTheProcess,subArraySize);
        printf(" -- ");
        localMax = maxValue(subArrayInTheProcess, subArraySize);
        printf("Maximum value in Process %d is %d \n", index + 1, localMax);
        write(pipeFromCtoP[index][WRITE_END], &localMax, sizeof(localMax));
        close(pipeFromCtoP[index][WRITE_END]); //closing the write end of child to parent pipe
    }
    //If the process is a parent process do the following:
    if (pid > 0){
        //=================Parent Proces Zone ===============
        randomizeArray(array, M);
        printf("Random elements in the inital array: ");
        printArrayElements(array,M);
        printf("\n");
        int firstIndexOfNextSubProcess = 0;
        //=================For processes with regular number of elements================
        for (int i = 0; i < numberOfProcesses ; i++)
        {
            int subProcessListElementAmount = elementAmounts[i];
            write(pipeFromPtoC[i][WRITE_END], &subProcessListElementAmount, sizeof(int));
            write(pipeFromPtoC[i][WRITE_END], &array[firstIndexOfNextSubProcess], sizeof(int) * subProcessListElementAmount );
            close(pipeFromPtoC[i][WRITE_END]); //closing the write end of parent to child pipe
            firstIndexOfNextSubProcess = firstIndexOfNextSubProcess + subProcessListElementAmount;
            printf("Sending Sublist to Process %d\n", i + 1);
        }
        //=================For processes with regular number of elements================
        sleep(5);
        //=================Retrive the results from child processes================
        int finalArray[MAX_ARRAY_SIZE];
        for (size_t i = 0; i < numberOfProcesses; i++)
        {
            int subProcessMax = 0;
            read(pipeFromCtoP[i][READ_END], &subProcessMax, sizeof(int));
            close(pipeFromCtoP[i][READ_END]);
            finalArray[i] = subProcessMax;
        }
        printf("Elements sent back to the parent process: ");        
        printArrayElements(finalArray,numberOfProcesses);
        printf("\n");        
        int globalMax = maxValue(finalArray, numberOfProcesses);
        printf("Global Maximum is %d \n", globalMax);
        //=================Retrive the results from child processes================
        //=================Parent Proces Zone ===============
    }
}
