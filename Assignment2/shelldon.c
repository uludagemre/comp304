/*
 * shelldon interface program

KUSIS ID: 50209 PARTNER NAME: Emre Uludağ 
KUSIS ID: 31760 PARTNER NAME: Arda Arslan

 */
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <wait.h>


#define MAX_LINE       80 /* 80 chars per line, per command, should be enough. */

int parseCommand(char inputBuffer[], char *args[],int *background, int *shouldRedirect, int *shouldAppend);

int main(void)
{
  char inputBuffer[MAX_LINE]; 	        /* buffer to hold the command entered */
  int background;             	        /* equals 1 if a command is followed by '&' */
  char *args[MAX_LINE/2 + 1];	        /* command line (of 80) has max of 40 arguments */
  pid_t child;            		/* process id of the child process */
  int status;           		/* result from execv system call*/
  int shouldrun = 1;
  int shouldRedirect;
  int shouldAppend;

	
  int i, upper;
		
  while (shouldrun){            		/* Program terminates normally inside setup */
    background = 0;
		
    shouldrun = parseCommand(inputBuffer,args,&background,&shouldRedirect,&shouldAppend);       /* get next command */
		
    if (strncmp(inputBuffer, "exit", 4) == 0)
      shouldrun = 0;     /* Exiting from shelldon*/


    //Below is the answer of the first part

    if (shouldrun) {
        int pid = fork();
        if (pid == 0) { // child
            char path[500];
            char filename[500];
            FILE* outputFile;
            int count = 0;
            strcpy(path, "/bin/");
            if ((shouldRedirect) && (shouldAppend)) {
              

              
              while (true) {
              
                if (strncmp(args[count], ">>", 2) == 0) {
              
                  strcpy(filename, args[count+1]);
                  args[count] = NULL;
                  break;
                }
                count++;
              }

              freopen(filename,"a+",stdout);
            }
             if ((shouldRedirect) && (!shouldAppend)) {
              while (true) {
                if (strncmp(args[count], ">", 1) == 0) {
                  strcpy(filename, args[count+1]);
                  args[count] = NULL;
                  break;
                }
                count++;
              }
              // printf("%s",stdout);
              freopen(filename,"w+",stdout);
            }
            strcat(path, args[0]);
            execv(path, args);
            // printf("%d\n",shouldRedirect);
            // printf("%d\n",shouldAppend);
          
        }
        else { // parent
            if (background == 1) {
                //This is the part when user puts '&' sign after the first command    
                wait(NULL);
            }
            //This is the part when user does not put '&' sign after the first command so there is no backgroung application is running 
        } 

      /*
	After reading user input, the steps are 
	(1) Fork a child process using fork()
	(2) the child process will invoke execv()
	(3) if command included &, parent will invoke wait()
       */
    }
  }
  return 0;
}

/** 
 * The parseCommand function below will not return any value, but it will just: read
 * in the next command line; separate it into distinct arguments (using blanks as
 * delimiters), and set the args array entries to point to the beginning of what
 * will become null-terminated, C-style strings. 
 */

int parseCommand(char inputBuffer[], char *args[],int *background, int *shouldRedirect, int *shouldAppend)
{
    int length,		/* # of characters in the command line */
      i,		/* loop index for accessing inputBuffer array */
      start,		/* index where beginning of next command parameter is */
      ct,	        /* index of where to place the next parameter into args[] */
      command_number;	/* index of requested command number */
    
    ct = 0;
	
    /* read what the user enters on the command line */
    do {
	  printf("shelldon>");
	  fflush(stdout);
	  length = read(STDIN_FILENO,inputBuffer,MAX_LINE); 
    }
    while (inputBuffer[0] == '\n'); /* swallow newline characters */
	
    /**
     *  0 is the system predefined file descriptor for stdin (standard input),
     *  which is the user's screen in this case. inputBuffer by itself is the
     *  same as &inputBuffer[0], i.e. the starting address of where to store
     *  the command that is read, and length holds the number of characters
     *  read in. inputBuffer is not a null terminated C-string. 
     */    
    start = -1;
    if (length == 0)
      exit(0);            /* ^d was entered, end of user command stream */
    
    /** 
     * the <control><d> signal interrupted the read system call 
     * if the process is in the read() system call, read returns -1
     * However, if this occurs, errno is set to EINTR. We can check this  value
     * and disregard the -1 value 
     */

    if ( (length < 0) && (errno != EINTR) ) {
      perror("error reading the command");
      exit(-1);           /* terminate with error code of -1 */
    }
    
    /**
     * Parse the contents of inputBuffer
     */
    
    for (i=0;i<length;i++) { 
      /* examine every character in the inputBuffer */
      
      switch (inputBuffer[i]){
      case ' ':
      case '\t' :               /* argument separators */
	if(start != -1){
	  args[ct] = &inputBuffer[start];    /* set up pointer */
	  ct++;
	}
	inputBuffer[i] = '\0'; /* add a null char; make a C string */
	start = -1;
	break;
	
      case '\n':                 /* should be the final char examined */
	if (start != -1){
	  args[ct] = &inputBuffer[start];     
	  ct++;
	}
	inputBuffer[i] = '\0';
	args[ct] = NULL; /* no more arguments to this command */
	break;
	
      default :             /* some other character */
	if (start == -1)
	  start = i;
	if (inputBuffer[i] == '&') {
	  *background  = 1;
	  inputBuffer[i-1] = '\0';
	}
      
  if ((i != 0) && (inputBuffer[i-1] != '>') && (inputBuffer[i] == '>') && (inputBuffer[i+1] != '>'))  {
    *shouldAppend = 0;
    *shouldRedirect = 1;
    
    
  }
  if ((i != 0) && (inputBuffer[i-1] != '>') && (inputBuffer[i] == '>') && (inputBuffer[i+1] == '>'))  {
    *shouldAppend = 1;
    *shouldRedirect = 1;
    
  }
  // if (strncmp(inputBuffer[i], ">>")){ 
  //   *shouldAppend = 0;
  //   *shouldRedirect = 1;
  // }
      
      
      } /* end of switch */
    }    /* end of for */
  
    /**
     * If we get &, don't enter it in the args array
     */
    
    if (*background)
      args[--ct] = NULL;
    
    args[ct] = NULL; /* just in case the input line was > 80 */
    
    return 1;
    
} /* end of parseCommand routine */