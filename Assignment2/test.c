// #include <stdio.h>
// #include <string.h>
// #include <stdlib.h>

// #define MAX_SIZE 20

// void printHistogram(char *histogramArray[], int histogramArraySize);
// int findElementIndex(char *searchedWord, char *history[], int historyCount);
// int hasTheWord(char *searchedWord, char *history[], int historyCount);
// int createHistogram(char *history[], char *histogram[], int historyCount);
// int main()
// {

  
//   char histogram[50][100] ;
//   int historycount = 8;
//   int histogramCount = 0;
//   for(size_t i = 0; i < historycount; i++)
//   {
//    if(findElementIndex(history[i],histogram,histogramCount)== -1){
//      strcpy(histogram[i],history[i]);
//    }
   
//   }
  
//   // system("crontab -l >> crontabFile");
//   // system("ls");
//   // system("cat crontabFile");

//   // int size = 8;
//   // char *name = "ali";
//   // int result = hasTheWord(name,histogram,count);
//   // printf("%s is contained: %d\n",name,result);
//   // int history_count = 10;
//   // for (int i = 1; i < history_count; i++)
//   // {
//   //   printf("I am here %d!\n", history_count);
//   // }

//   // printf("This is the index %d  \n", findElementIndex("serhat", histogram, size));
//   // this is my histogram implementation
  

//     // for(int i = 1; i < history_count; i++)
//     // {
//     //   printf("I am here %d!\n",history_count);

//     //   }else{
//     //     int index = findElementIndex(history[i],histogramArray,histogramArraySize);
//     //     strcat(histogramArray[index+1],"*");

//     //   }

//     // }

   
//   //  printHistogram(&histogram,histogramCount);


//   // }

//   return 0;
// }

// int hasTheWord(char *searchedWord, char* history[], int historyCount)
// {
//   int found = 0;
//   int i = 0;
//   while (i < historyCount & history[0] != NULL )
//   {
//     if (strcmp(searchedWord, history[i]) == 0)
//     {
//       found = 1;
//       return found;
//     };
//     i++;
//   }
//   return found;
// }
// int findElementIndex(char *searchedWord, char *history[], int historyCount)
// {
//   int i = 0;
//   while (i < historyCount)
//   {
//     if (strcmp(searchedWord, history[i]) == 0)
//     {
//       return i;
//     }
//     else
//     {
//       i++;
//     }
//   }
//   return -1;
// }
// void printHistogram(char *histogramArray[], int histogramArraySize)
// {
//   for (int i = 0; i < histogramArraySize; i++)
//   {
//     if (i % 2 == 0)
//     {
//       printf("%s :", histogramArray[i]);
//     }
//     else
//     {
//       printf("%s\n", histogramArray[i]);
//     }
//   }
// }
// // int createHistogram(char *history[], char *histogram[50], int historyCount)
// // {
// //   int i = 0;
// //   int j = 0;
// //   int histogramSize = 0;
// //   char **ptr;
// //   ptr = history;
// //   while(i < historyCount){
      
// //         int index = findElementIndex(history[i], histogram, histogramSize);
// //         strcat(histogram[index+1],"*");
// //       }
// //     }
// // return histogramSize;
// // }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct node{
    int key;
    char* val;
    char* histogramStar;
    struct node *next;
};
struct table{
    int size;
    struct node **list;
};
struct table *createTable(int size){
    struct table *t = (struct table*)malloc(sizeof(struct table));
    t->size = size;
    t->list = (struct node**)malloc(sizeof(struct node*)*size);
    int i;
    for(i=0;i<size;i++)
        t->list[i] = NULL;
    return t;
}
int hashCode(struct table *t,int key){
    if(key<0)
        return -(key%t->size);
    return key%t->size;
}
void insert(struct table *t,int key,char* val, char* histogramStar){
    int pos = hashCode(t,key);
    struct node *list = t->list[pos];
    struct node *newNode = (struct node*)malloc(sizeof(struct node));
    struct node *temp = list;
    while(temp){
        if(temp->key==key){
            strcpy(val,temp->val);
            return;
        }
        temp = temp->next;
    }
    newNode->key = key;
    newNode->val = val;
    newNode->histogramStar = histogramStar;
    newNode->next = list;
    t->list[pos] = newNode;
}
char* lookupCommandName(struct table *t,int key){
    int pos = hashCode(t,key);
    struct node *list = t->list[pos];
    struct node *temp = list;
    while(temp){
        if(temp->key==key){
            return temp->val;
        }
        temp = temp->next;
    }
    return NULL;
}
char* lookupHistogramStar(struct table *t,int key){
    int pos = hashCode(t,key);
    struct node *list = t->list[pos];
    struct node *temp = list;
    while(temp){
        if(temp->key==key){
            return temp->histogramStar;
        }
        temp = temp->next;
    }
    return NULL;
}
int main(){
    struct table *t = createTable(100);
    char *history[] = {"emre", "ali", "mehmet", "emre", "ahmet", "yonca", "serhat", "defne"};
    int tableSize = 0;
    for(size_t i = 0; i < sizeof(history)/sizeof(history[0]); i++)
    {
    insert(t,i,history[i],"*");
    tableSize++;
    }
    
    for(int i = 0; i < tableSize; i++)
    {
      printf("%d : %s - %s\n",i+1,lookupCommandName(t,i),lookupHistogramStar(t,i));
    }
    
    


    return 0;
}
