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
#include <dirent.h>
#include <signal.h>
#define SIZE 40
#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */


int hasTheWord(char* searchedWord,char* history[],int historyCount);
void printHistogram(char* histogramArray[],int histogramArraySize);
int findElementIndex(char* searchedWord,char* history[],int historyCount);

void codesearch(const char *name, char *search_str, bool is_recursive, char *forced_filename);
int parseCommand(char inputBuffer[], char *args[], int *background, int *shouldRedirect, int *shouldAppend, int isInHistory, char historyCommand[]);

int main(void)
{
  char inputBuffer[MAX_LINE];   /* buffer to hold the command entered */
  int background;               /* equals 1 if a command is followed by '&' */
  char *args[MAX_LINE / 2 + 1]; /* command line (of 80) has max of 40 arguments */
  pid_t child;                  /* process id of the child process */
  int status;                   /* result from execv system call*/
  int shouldrun = 1;
  int shouldRedirect;
  int shouldAppend;
  FILE *mystdout = stdout;
  char history[100][50];
  int history_count = 0;
  char historyCommand[MAX_LINE];
  int isInHistory;

  int i, upper;
  int first_time = 1;
  int first_time_2 = 1;

  while (shouldrun)
  { /* Program terminates normally inside setup */

    background = 0;
    shouldRedirect = 0;
    shouldAppend = 0;
    first_time_2 = 1;

    if (first_time != 1)
    {
      fclose(stdout);
    }
    else
    {
      first_time = 1;
    }
    stdout = mystdout;

    shouldrun = parseCommand(inputBuffer, args, &background, &shouldRedirect, &shouldAppend, isInHistory, historyCommand); /* get next command */

    char **ptr = args;
    char current_command[MAX_LINE];
    strcpy(current_command, "");
    while (*ptr != NULL)
    {
      if (first_time_2 != 1)
      {
        strcat(current_command, " ");
        strcat(current_command, *ptr);
      }
      else
      {
        strcpy(current_command, *ptr);
        first_time_2 = 0;
      }
      *ptr++;
    }
    if (background == 1)
    {
      strcat(current_command, " &");
    }

    strcat(current_command, "\0");
    if (!((args[0][0] == '!') || (args[0][1] == '!')))
    {
      strcpy(history[history_count], current_command);
      history_count++;
    }

    if (strncmp(inputBuffer, "exit", 4) == 0)
      shouldrun = 0; /* Exiting from shelldon*/

    //Below is the answer of the first part

    if (shouldrun)
    {
      child = fork();
      if (child == 0)
      { // child
        char path[500];
        char filename[500];

        int count = 0;

        if ((shouldRedirect == 1) && (shouldAppend == 1))
        {
          while (true)
          {
            if (strncmp(args[count], ">>", 2) == 0)
            {
              strcpy(filename, args[count + 1]);
              args[count] = NULL;
              break;
            }
            count++;
          }

          freopen(filename, "a+", stdout);
        }
        if ((shouldRedirect == 1) && (shouldAppend == 0))
        {
          while (true)
          {
            if (strncmp(args[count], ">", 1) == 0)
            {
              strcpy(filename, args[count + 1]);
              args[count] = NULL;
              break;
            }
            count++;
          }
          freopen(filename, "w+", stdout);
        }

        if (strcmp(args[0], "gcc") == 0)
        {
          strcpy(path, "/usr/bin/");
        }
        else if (strcmp(args[0], "cd") == 0)
        {
          exit(0);
        }
        else if ((strcmp(args[0], "codesearch") == 0) && (args[1] != NULL)) {
          if (strcmp(args[1], "-r") == 0) {
            char* full_search_str = args[2];
            int length = strlen(full_search_str) - 2;
            int position = 1;
            char search_str[1024];
            int c = 0;
            while (c < length) {
                search_str[c] = full_search_str[position+c];
                c++;
            }
            search_str[c] = '\0';
            codesearch(".", search_str, true, NULL);
          }
          else if ((args[2] != NULL) && (strcmp(args[2], "-f") == 0)) {
            char* full_search_str = args[1];
            int length = strlen(full_search_str) - 2;
            int position = 1;
            char search_str[1024];
            int c = 0;
            while (c < length) {
                search_str[c] = full_search_str[position+c];
                c++;
            }
            search_str[c] = '\0';
            codesearch(".", search_str, true, args[3]); //TODO
          }
          else {
            char* full_search_str = args[1];
            int length = strlen(full_search_str) - 2;
            int position = 1;
            char search_str[1024];
            int c = 0;
            while (c < length) {
                search_str[c] = full_search_str[position+c];
                c++;
            }
            search_str[c] = '\0';
            codesearch(".", search_str, false, NULL);
          }
        }
        else if (strcmp(args[0], "birdakika") == 0)
        {
        
          char *timeArgument = args[1];//This is the time in the form HH.MM
          char *musicFileName = args[2]; 
          char timeArray[2][5];
          char *p;
          int i = 0;
          p = strtok(timeArgument,".");
          while (p != NULL)
          {
            strcpy(timeArray[i],p);
            p = strtok(NULL, ".");
            i++;
          }
          char *musicfile = args[2];


          FILE *fpMusic;
          fpMusic = fopen("/home/user/Desktop/comp304/Assignment2/play.sh", "w");
          fprintf(fpMusic, "play /home/user/Desktop/comp304/Assignment2/%s trim 0.0 60",musicFileName);
          fclose(fpMusic);

          FILE *fpCrontab;
          fpCrontab = fopen("/var/spool/cron/crontabs/user.bin", "wb");
          fprintf(fpCrontab, "%s %s * * * /home/user/Desktop/comp304/Assignment2/play.sh\n",timeArray[1],timeArray[0]);
          fclose(fpCrontab);
       
        }
        else if (args[0][0] == '!') //If you choose to redirect an old statement you enter this if statement
        {

          if (args[0][1] != '!')
          {

            int length = strlen(args[0]);
            char subbuff[5];
            memcpy(subbuff, &args[0][1], length);
            subbuff[length] = '\0';
            int targetHistory = atoi(subbuff);
            strcpy(historyCommand, history[targetHistory - 1]);
            isInHistory = 1;
            strcat(historyCommand, "\n\0");
            char path[20];
            shouldrun = parseCommand(inputBuffer, args, &background, &shouldRedirect, &shouldAppend, isInHistory, historyCommand);
            if (strcmp(args[0], "history") == 0)
            {

              if (history_count == 1)
              {
                printf("No history data found \n");
                break;
              }

              int historyEndingIndex = history_count - 1;
              int index = history_count;
              printf("\nLast commands: \n");
              for (int i = historyEndingIndex; (i > historyEndingIndex - 10) & (i >= 0); i--) //print at most last 10 elements in the histroy
              {
                printf("%d %s\n", index, history[i]);
                index--;
              }
            }
            else if (strcmp(args[0], "cd") == 0)
            {
              chdir(args[1]);
            }
            else if ((strcmp(args[0], "codesearch") == 0) && (args[1] != NULL))
            {
              if (strcmp(args[1], "-r") == 0)
              {
                exit(0);
              }
              else if (strcmp(args[1], "-f") == 0)
              {
                exit(0);
              }
              else
              {
                exit(0);
              }
            }
            else
            {
              strcpy(path, "/bin/");
              strcat(path, args[0]);
              execv(path, args);
            }
          }
          else //This is inside !!
          {
            strcpy(historyCommand, history[history_count - 1]);
            isInHistory = 1;
            strcat(historyCommand, "\n\0");
            char path[20];
            printf("\n%s", historyCommand);
            shouldrun = parseCommand(inputBuffer, args, &background, &shouldRedirect, &shouldAppend, isInHistory, historyCommand);
            if (strcmp(args[0], "history") == 0)
            {

              if (history_count == 1)
              {
                printf("No history data found \n");
                break;
              }

              int historyEndingIndex = history_count - 1;
              int index = history_count;
              printf("\nLast commands: \n");
              for (int i = historyEndingIndex; (i > historyEndingIndex - 10) & (i >= 0); i--) //print at most last 10 elements in the histroy
              {
                printf("%d %s\n", index, history[i]);
                index--;
              }
            }
            else if (strcmp(args[0], "cd") == 0)
            {
              chdir(args[1]);
            }
            else
            {
              strcpy(path, "/bin/");
              strcat(path, args[0]);
              printf("this is path %s", path);
              execv(path, args);
            }
          }
        }

        else if (strcmp(args[0], "history") == 0)
        {

          if (history_count == 1)
          {
            printf("No history data found \n");
            break;
          }

          int historyEndingIndex = history_count - 1;
          int index = history_count;
          printf("\nLast commands: \n");
          for (int i = historyEndingIndex; (i > historyEndingIndex - 10) & (i >= 0); i--) //print at most last 10 elements in the histroy
          {
            printf("%d %s\n", index, history[i]);
            index--;
          }
        }
        else if (strcmp(args[0], "histogram") == 0){
            char* histogramArray[100];
            histogramArray[0]= history[0];
            histogramArray[1]="*";
            int histogramArraySize = 2;
            int j=2;
            
            for(int i = 1; i < history_count; i++)
            {
              printf("I am here %d!\n",history_count);
              
              if(!hasTheWord(history[i],histogramArray,histogramArraySize)){
                histogramArray[j]=history[i];
                histogramArray[j+1]="*";
                histogramArraySize++;
                j+=2;
               

              }else{
                int index = findElementIndex(history[i],histogramArray,histogramArraySize);
                strcat(histogramArray[index+1],"*");
               
              }

            }
            printHistogram(histogramArray,histogramArraySize);
            
            
        }
        else
        {
          strcpy(path, "/bin/");
        }

        strcat(path, args[0]);
        execv(path, args);

        exit(0);
      }
      else
      { // parent
        if (background == 1)
        {
          //This is the part when user puts '&' sign after the first command
          wait(NULL);
        }

        if (strcmp(args[0], "cd") == 0)
        {
          chdir(args[1]);
        }
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

void codesearch(const char *name, char *search_str, bool is_recursive, char *forced_filename)
{

  DIR *dir;
  struct dirent *entry;

  if (!(dir = opendir(name)))
    return;

  while ((entry = readdir(dir)) != NULL)
  {
    if (entry->d_type == DT_DIR)
    {
      char dir_name[1024];
      if (!is_recursive || strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        continue;

      // snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
      // printf("%s\n", entry->d_name);
      // printf("This is a path: %s/%s\n", name, entry->d_name);
      sprintf(dir_name, "%s/%s", name, entry->d_name);
      // printf("This is a directory: %s\n", dir_name);
      codesearch(dir_name, search_str, is_recursive, forced_filename);
    }
    else
    {
      char file_name[1024];
      int count = 1;
      sprintf(file_name, "%s/%s", name, entry->d_name);
      if (forced_filename == NULL || strcmp(forced_filename, file_name) == 0)
      {
        FILE *file = fopen(file_name, "r"); /* should check the result */
        char line[1024];
        while (fgets(line, sizeof(line), file))
        {
          /* note that fgets don't strip the terminating \n, checking its
                        presence would allow to handle lines longer that sizeof(line) */
          if (strstr(line, search_str) != NULL)
          {
            printf("%d: %s -> %s", count, file_name, line);
          }
          count++;
        }
        fclose(file);
        // printf("%s\n", file_name);
      }
      /* may check feof here to make a difference between eof and io failure -- network
                timeout for instance */
    }
  }
  closedir(dir);

    if (forced_filename != NULL) {
      int count = 1;
      
      FILE* file = fopen(forced_filename, "r"); /* should check the result */
      char line[1024];
      while (fgets(line, sizeof(line), file)) {
          /* note that fgets don't strip the terminating \n, checking its
              presence would allow to handle lines longer that sizeof(line) */
          if(strstr(line, search_str) != NULL) {
              printf("%d: %s -> %s", count, forced_filename, line);     
          }
          count++;
      }
      fclose(file);
      // printf("%s\n", file_name);
      /* may check feof here to make a difference between eof and io failure -- network
          timeout for instance */
    }
    else {
      DIR *dir;
      struct dirent *entry;
      
      if (!(dir = opendir(name)))
        return;

      while ((entry = readdir(dir)) != NULL) {
          if (entry->d_type == DT_DIR) {
              char dir_name[1024];
              if (!is_recursive || strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                  continue;
              
              // snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
              // printf("%s\n", entry->d_name);
              // printf("This is a path: %s/%s\n", name, entry->d_name);
              sprintf(dir_name, "%s/%s", name, entry->d_name);
              // printf("This is a directory: %s\n", dir_name);
              codesearch(dir_name, search_str, is_recursive, forced_filename);
          } else {
              char file_name[1024];
              int count = 1;
              sprintf(file_name, "%s/%s", name, entry->d_name);
              
              FILE* file = fopen(file_name, "r"); /* should check the result */
              char line[1024];
              while (fgets(line, sizeof(line), file)) {
                  /* note that fgets don't strip the terminating \n, checking its
                      presence would allow to handle lines longer that sizeof(line) */
                  if(strstr(line, search_str) != NULL) {
                      printf("%d: %s -> %s", count, file_name, line);     
                  }
                  count++;
              }
              fclose(file);
              // printf("%s\n", file_name);
              /* may check feof here to make a difference between eof and io failure -- network
                  timeout for instance */
          }
      }
      closedir(dir);
    }
}

int parseCommand(char inputBuffer[], char *args[], int *background, int *shouldRedirect, int *shouldAppend, int isInHistory, char historyCommand[])
{
  int length,         /* # of characters in the command line */
      i,              /* loop index for accessing inputBuffer array */
      start,          /* index where beginning of next command parameter is */
      ct,             /* index of where to place the next parameter into args[] */
      command_number; /* index of requested command number */

  ct = 0;

  /* read what the user enters on the command line */

  if (isInHistory == 0)
  {
    do
    {
      printf("shelldon>");
      fflush(stdout);
      length = read(STDIN_FILENO, inputBuffer, MAX_LINE);
    } while (inputBuffer[0] == '\n'); /* swallow newline characters */
  }
  else
  {

    strcpy(inputBuffer, historyCommand);
    int tempLength = strlen(inputBuffer);
    length = strlen(inputBuffer);
    isInHistory = 0;

    //You must drop the thing down
  }

  /**
     *  0 is the system predefined file descriptor for stdin (standard input),
     *  which is the user's screen in this case. inputBuffer by itself is the
     *  same as &inputBuffer[0], i.e. the starting address of where to store
     *  the command that is read, and length holds the number of characters
     *  read in. inputBuffer is not a null terminated C-string. 
     */
  start = -1;
  if (length == 0)
    exit(0); /* ^d was entered, end of user command stream */

  /** 
     * the <control><d> signal interrupted the read system call 
     * if the process is in the read() system call, read returns -1
     * However, if this occurs, errno is set to EINTR. We can check this  value
     * and disregard the -1 value 
     */

  if ((length < 0) && (errno != EINTR))
  {
    perror("error reading the command");
    exit(-1); /* terminate with error code of -1 */
  }

  /**
     * Parse the contents of inputBuffer
     */

  for (i = 0; i < length; i++)
  {
    /* examine every character in the inputBuffer */

    switch (inputBuffer[i])
    {
    case ' ':
    case '\t': /* argument separators */
      if (start != -1)
      {
        args[ct] = &inputBuffer[start]; /* set up pointer */
        ct++;
      }
      inputBuffer[i] = '\0'; /* add a null char; make a C string */
      start = -1;
      break;

    case '\n': /* should be the final char examined */
      if (start != -1)
      {
        args[ct] = &inputBuffer[start];
        ct++;
      }
      inputBuffer[i] = '\0';
      args[ct] = NULL; /* no more arguments to this command */
      break;

    default: /* some other character */
      if (start == -1)
        start = i;
      if (inputBuffer[i] == '&')
      {
        *background = 1;
        inputBuffer[i - 1] = '\0';
      }

      if ((i != 0) && (inputBuffer[i - 1] != '>') && (inputBuffer[i] == '>') && (inputBuffer[i + 1] != '>'))
      {
        *shouldAppend = 0;
        *shouldRedirect = 1;
      }
      if ((i != 0) && (inputBuffer[i - 1] != '>') && (inputBuffer[i] == '>') && (inputBuffer[i + 1] == '>'))
      {
        *shouldAppend = 1;
        *shouldRedirect = 1;
      }
      // if (strncmp(inputBuffer[i], ">>")){
      //   *shouldAppend = 0;
      //   *shouldRedirect = 1;
      // }

    } /* end of switch */
  }   /* end of for */

  /**
     * If we get &, don't enter it in the args array
     */

  if (*background)
    args[--ct] = NULL;
  args[ct] = NULL; /* just in case the input line was > 80 */
  // strcpy(inputBuffer,"");
  return 1;

} /* end of parseCommand routine */

int hasTheWord(char* searchedWord,char* history[],int historyCount){
  int found=0;
  int i= 0;
  while (i < historyCount) {
    if(strcmp(searchedWord,history[i])==0){
        found = 1;
        return found;
    };
    i++;
  }
return found;
}

int findElementIndex(char* searchedWord,char* history[],int historyCount){
  int i= 0;
  while (i < historyCount) {
    if(strcmp(searchedWord,history[i])==0){
        return i;
    }
    else{
        i++;
    } 
  }
return -1;
}
 void printHistogram(char* histogramArray[],int histogramArraySize){
    for(int i = 0; i < histogramArraySize; i++)
    {
      if(i%2 == 0){
        printf("%s :",histogramArray[i]);
      }else{
        printf("%s\n",histogramArray[i]);
      }
    }
    
 }