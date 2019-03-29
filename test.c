
#include <stdio.h>
#include <string.h>

int main(void)
{

    char *numbers[] = {"One", "Two", "Three", NULL}, **n;

    n = numbers;
    // while (strcmp(*n,"")!=0) {
    while(*n != NULL){
    printf ("%s\n",  *n++);
      }
    
    
return 0;
}