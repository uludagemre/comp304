/*
 * shelly interface program

KUSIS ID:53572 PARTNER NAME:BURAK
KUSIS ID:49839 PARTNER NAME:IRMAK

 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>


#define MAX_LINE       80 /* 80 chars per line, per command, should be enough. */

int parseCommand(char inputBuffer[], char *args[],int *background);
int execvp(const char *path, char *const params[]);
pid_t waitpid(pid_t pid,int *status,int options);

int main(void)
{
  char inputBuffer[MAX_LINE]; 	        /* buffer to hold the command entered */
  int background;             	        /* equals 1 if a command is followed by '&' */
  char *args[MAX_LINE/2 + 1];	        /* command line (of 80) has max of 40 arguments */
  pid_t child;            		/* process id of the child process */
  int status;           		/* result from execv system call*/
  int shouldrun = 1;
  int i, upper;
//my additional variables
FILE* fscript;
FILE* fbookmark;
char  *key[MAX_LINE];
char  *value[MAX_LINE];
char weather[MAX_LINE];
time_t mytime;
char * times;
//script command works on its own, bookmark commands on their own and time I/0,and weather work together.

	
  while (shouldrun){            		/* Program terminates normally inside setup */
    background = 0;
	
    shouldrun = parseCommand(inputBuffer,args,&background);       /* get next command */

    if (strncmp(inputBuffer, "exit", 4) == 0){
      shouldrun = 0;     /* Exiting from myshell*/

    }else if(strncmp(inputBuffer,"script",4) == 0){//Part 2 Script Command, does not work with bookmark or I/O for I/O comment all 							additional commands
	fscript=fopen(args[1],"w+");
	
	i=0;
	while(args[i] != NULL){

	fputs(args[i] ,fscript);
	fputs("\t" ,fscript);
	i++;
	}
	fputs("\n" ,fscript);

    }else if(strncmp(inputBuffer,"bookmark",4) == 0){

	fbookmark=fopen("bookmarks.txt","w");
	
	fputs(args[1],fbookmark);
	fputs("\t" ,fbookmark);
	fputs(args[2],fbookmark);
	fputs("\n" ,fbookmark);
	key[0]=args[1];
	value[0]=args[2];
	printf("key \t %s \n",key[0]);
	printf("val \t %s \n",value[0]);
	

	}else if(strncmp (inputBuffer,key[0],4) == 0){
	execvp(args[0],args);
	exit(0);

	
	}else if(strncmp (inputBuffer,"-r",4) == 0){
		
	key[0]=NULL;
	value[0]=NULL;
	printf("key \t %s \n",key[0]);
	printf("val \t %s \n",value[0]);

   	}else if(strncmp(inputBuffer,"time",4) == 0){//my command displays the time
	mytime = time(NULL);
	times = ctime(&mytime);
	times[strlen(times)-1] = '\0';
	printf("Time is \t %s ",times);
	exit(0);

    	}else if(strncmp(inputBuffer,"wforecast",4) == 0){
	strcpy(weather,"echo '0 9 * * * curl wttr.in/Istanbul.png > ");
	strcat(weather,args[1]);//Adds the file
	strcat(weather,"' | crontab -");
	system(weather);//weather is executed by this 
	
   }
//1st Part of the Assignment
    if (shouldrun) {

	child=fork();
	if(child!=0){
		if(&background ==0){ //checks if background process or not
	waitpid(child,&status,0);
	exit(0);
}
	}else{

	child=getpid();
	execv(args[0],args);

 	//kill(1); //uncomment while script
	exit(0);   //uncomment while bookmark

	} 
	
      /*
	After reading user input, the steps are 
	(1) Fork a child process using fork()
	(2) the child process will invoke execv()
	(3) if command included &, parent will invoke wait()
       */
    
  } }
  return 0;

}

/** 
 * The parseCommand function below will not return any value, but it will just: read
 * in the next command line; separate it into distinct arguments (using blanks as
 * delimiters), and set the args array entries to point to the beginning of what
 * will become null-terminated, C-style strings. 
 */

int parseCommand(char inputBuffer[], char *args[],int *background)
{
    int length,		/* # of characters in the command line */
      i,		/* loop index for accessing inputBuffer array */
      start,		/* index where beginning of next command parameter is */
      ct,	        /* index of where to place the next parameter into args[] */
      command_number;	/* index of requested command number */
    int redirection;
    ct = 0;
    FILE* output;

	
    /* read what the user enters on the command line */
    do {
	  printf("shelly>");
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

	//I/O redirection below 
	if (inputBuffer[i] == '>') {
		if(inputBuffer[i+1] != '>'){
	  redirection  = 1;
	  ct++;
	  	 
	output=fopen(args[ct-2],"w+");//truncating part
	output=stdout;
		}else if(inputBuffer[i+1] == '>'){
		 redirection  = 0;
		 ct++; 
	
		output=fopen(args[ct-2],"a+");//appending part
			
	}
	}
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


