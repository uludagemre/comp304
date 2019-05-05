#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <queue>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <fstream>



pthread_mutex_t fill_mutex;
int arr[10];
int flag = 0;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;

void *fill(void* data)
{
    int counter = 0;
    while (1 > 0) {
        pthread_mutex_lock(&fill_mutex);
        pthread_cond_wait(&cond_var, fill_mutex);
        arr[counter] = counter;
        counter++;
        pthread_mutex_unlock(&fill_mutex);
        pthread_sleep(0.1);
    }
    
    pthread_cond_signal(&cond_var);
    
    pthread_exit(NULL);
}

void *read()
{
    int i;
    pthread_mutex_lock(&fill_mutex);
    pthread_cond_wait(&cond_var, &fill_mutex);
    pthread_mutex_unlock(&fill_mutex);
    printf("Values filled in array are");
    for (i = 0; i < 10; i++)
    {
        printf("\n %d \n", arr[i]);
    }
    pthread_exit(NULL);
}

int pthread_sleep(int seconds)
{
    pthread_mutex_t mutex;
    pthread_cond_t conditionvar;
    struct timespec timetoexpire;
    if (pthread_mutex_init(&mutex, NULL))
    {
        return -1;
    }
    if (pthread_cond_init(&conditionvar, NULL))
    {
        return -1;
    }
    struct timeval tp;
    //When to expire is an absolute time, so get the current time and add //it to our delay time
    gettimeofday(&tp, NULL);
    timetoexpire.tv_sec = tp.tv_sec + seconds;
    timetoexpire.tv_nsec = tp.tv_usec * 1000;

    pthread_mutex_lock(&mutex);
    int res = pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&conditionvar);

    //Upon successful completion, a value of zero shall be returned
    return res;
}

main()
{
    int i;
    pthread_t thread_id, thread_id1;
    pthread_attr_t attr;
    int ret;
    void *res;
    ret = pthread_create(&thread_id, NULL, &fill, NULL);
    // ret = pthread_create(&thread_id1, NULL, &read, NULL);
    pthread_join(thread_id, &res);
    // pthread_join(thread_id1, &res);

    sleep(1);

    for (i = 0; i < 10; i++)
    {
        printf("%d \n", arr[i]);
    }
}