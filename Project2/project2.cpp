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
using namespace std;

struct Car {
    int id;
    char incoming_direction;
    time_t arrival_time;
    time_t cross_time;
    float waiting_time;
};

pthread_t thread_N;
pthread_t thread_E;
pthread_t thread_S;
pthread_t thread_W;
pthread_t thread_police;
pthread_mutex_t access_a_queue_mutex;
pthread_attr_t thread_attribute;

time_t end_time;
time_t current_time_N;
time_t current_time_E;
time_t current_time_S;
time_t current_time_W;
time_t current_time_police;

queue <Car> car_queue_N;
queue <Car> car_queue_E;
queue <Car> car_queue_S;
queue <Car> car_queue_W;

char id_direction_mapping[4] = {'N','E','S','W'};

float prob;
int num_cars = 0;

int pthread_sleep (int seconds)
{
   pthread_mutex_t mutex;
   pthread_cond_t conditionvar;
   struct timespec timetoexpire;
   if(pthread_mutex_init(&mutex,NULL))
    {
      return -1;
    }
   if(pthread_cond_init(&conditionvar,NULL))
    {
      return -1;
    }
   struct timeval tp;
   //When to expire is an absolute time, so get the current time and add //it to our delay time
   gettimeofday(&tp, NULL);
   timetoexpire.tv_sec = tp.tv_sec + seconds; timetoexpire.tv_nsec = tp.tv_usec * 1000;

   pthread_mutex_lock (&mutex);
   int res = pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
   pthread_mutex_unlock (&mutex);
   pthread_mutex_destroy(&mutex);
   pthread_cond_destroy(&conditionvar);

   //Upon successful completion, a value of zero shall be returned
   return res;
}

queue <Car> * get_queue(char direction) {
    if (direction == 'N') {
        return &car_queue_N;
    }
    else if (direction == 'E') {
        return &car_queue_E;
    }
    else if (direction == 'S') {
        return &car_queue_S;
    }
    else if (direction == 'W') {
        return &car_queue_W;
    }
}

time_t * get_time(char direction) {
    if (direction == 'N') {
        return &current_time_N;
    }
    else if (direction == 'E') {
        return &current_time_E;
    }
    else if (direction == 'S') {
        return &current_time_S;
    }
    else if (direction == 'W') {
        return &current_time_W;
    }
}

void add_new_car(char direction) {
    struct Car new_car;
    new_car.id = num_cars;
    new_car.incoming_direction = direction;
    new_car.arrival_time = time(NULL);
    new_car.waiting_time = 0.0;
    queue <Car> * car_queue = get_queue(direction);
    (*car_queue).push(new_car);
    num_cars = num_cars + 1;
}

void remove_first_car(char direction) {
    queue <Car> * car_queue = get_queue(direction);
    struct Car car_to_be_removed = (*car_queue).front();
    car_to_be_removed.cross_time = time(NULL);
    car_to_be_removed.waiting_time = (float) difftime(car_to_be_removed.cross_time, car_to_be_removed.arrival_time);
    (*car_queue).pop();
}

void update_waiting_times(char direction, int seconds_passed) {
    queue <Car> * queue_car = get_queue(direction);
    int queue_size = (*queue_car).size();
    queue <Car> result;
    for (int i=0; i<queue_size; i++) {
        struct Car current_car = (*queue_car).front();
        time_t current_time = time(NULL);
        current_car.waiting_time = current_car.waiting_time + (float)seconds_passed;
        (*queue_car).pop();
        result.push(current_car);
    }
    *queue_car = result;
}

bool is_there_5_or_more_cars_in_other_queues(char direction) {
    queue <Car> * queue_N = get_queue('N');
    queue <Car> * queue_E = get_queue('E');
    queue <Car> * queue_S = get_queue('S');
    queue <Car> * queue_W = get_queue('W');
    if ((direction != 'N') && ((*queue_N).size() >= 5)) {
        return true;
    }
    else if ((direction != 'E') && ((*queue_E).size() >= 5)) {
        return true;
    }
    else if ((direction != 'S') && ((*queue_S).size() >= 5)) {
        return true;
    }
    else if ((direction != 'W') && ((*queue_W).size() >= 5)) {
        return true;
    }
    return false;
}

char get_other_direction_with_maximum_number_of_cars(char direction) {
    queue <Car> * queue_N = get_queue('N');
    queue <Car> * queue_E = get_queue('E');
    queue <Car> * queue_S = get_queue('S');
    queue <Car> * queue_W = get_queue('W');

    int length_of_longest_queue = -1;
    char direction_of_longest_queue = 'X';

    if ((direction != 'N') && ((*queue_N).size() > length_of_longest_queue)) {
        length_of_longest_queue = (*queue_N).size();
        direction_of_longest_queue = 'N';
    }
    if ((direction != 'E') && ((*queue_E).size() > length_of_longest_queue)) {
        length_of_longest_queue = (*queue_E).size();
        direction_of_longest_queue = 'E';
    }
    if ((direction != 'S') && ((*queue_S).size() > length_of_longest_queue)) {
        length_of_longest_queue = (*queue_S).size();
        direction_of_longest_queue = 'S';
    }
    if ((direction != 'W') && ((*queue_W).size() > length_of_longest_queue)) {
        length_of_longest_queue = (*queue_W).size();
        direction_of_longest_queue = 'W';
    }
    if ((length_of_longest_queue == 0) || (length_of_longest_queue == 0)) {
        return 'X';
    }
    return direction_of_longest_queue;
}

char get_other_direction_with_a_car_waiting_for_more_than_20_seconds(char direction) {
    queue <Car> * queue_N = get_queue('N');
    queue <Car> * queue_E = get_queue('E');
    queue <Car> * queue_S = get_queue('S');
    queue <Car> * queue_W = get_queue('W');

    int length_of_longest_waiting = -1;
    char direction_of_queue_with_longest_waiting = 'X';

    if ((direction != 'N') && ((*queue_N).size() > 0) && ((*queue_N).front().waiting_time > length_of_longest_waiting)) {
        length_of_longest_waiting = ((*queue_N).front()).waiting_time;
        direction_of_queue_with_longest_waiting = 'N';
    }
    if ((direction != 'E') && ((*queue_E).size() > 0) && ((*queue_E).front().waiting_time > length_of_longest_waiting)) {
        length_of_longest_waiting = ((*queue_E).front()).waiting_time;
        direction_of_queue_with_longest_waiting = 'E';
    }
    if ((direction != 'S') && ((*queue_S).size() > 0) && ((*queue_S).front().waiting_time > length_of_longest_waiting)) {
        length_of_longest_waiting = ((*queue_S).front()).waiting_time;
        direction_of_queue_with_longest_waiting = 'S';
    }
    if ((direction != 'W') && ((*queue_W).size() > 0) && ((*queue_W).front().waiting_time > length_of_longest_waiting)) {
        length_of_longest_waiting = ((*queue_W).front()).waiting_time;
        direction_of_queue_with_longest_waiting = 'W';
    }
    if (length_of_longest_waiting > 20) {
        return direction_of_queue_with_longest_waiting;
    }
    return 'X';
}

void *car_thread_fn(void *direction_star) {
    char direction = id_direction_mapping[(long)direction_star];
    int is_n_trap = 0;

    time_t * current_thread_time = get_time(direction);
    *current_thread_time = time(NULL);

    while (*current_thread_time < end_time) {
        pthread_mutex_lock(&access_a_queue_mutex);

        if ((direction == 'E') || (direction == 'S') || (direction == 'W')) {
            if ((rand() / (float)RAND_MAX) < prob) {
                add_new_car(direction);
            }
        }
        else if (direction == 'N') {
            if ((rand() / (float)RAND_MAX) < 1.0 - prob) {
                add_new_car(direction);
            }
            else {
                is_n_trap = 1;
            }
        }
        pthread_mutex_unlock(&access_a_queue_mutex);
        if (is_n_trap == 1) {
            pthread_sleep(1); // Change to 20 after debugging.
        }
        else {
            pthread_sleep(1);
        }

        time_t * current_thread_time = get_time(direction);
        *current_thread_time = time(NULL);
    }
    pthread_exit(0);
}

void *police_thread_fn(void *sth) {
    current_time_police = time(NULL);
    char direction = 'N';
    char current_direction;
    int removed_car;
    while (current_time_police < end_time) {
        pthread_mutex_lock(&access_a_queue_mutex);
        
        if (get_other_direction_with_a_car_waiting_for_more_than_20_seconds(direction) != 'X') {
            direction = get_other_direction_with_a_car_waiting_for_more_than_20_seconds(direction);
            remove_first_car(direction);
            removed_car = 1;
        }
        else if ((get_other_direction_with_maximum_number_of_cars(direction) != 'X') &&
                 (is_there_5_or_more_cars_in_other_queues(direction) == true)) {
            direction = get_other_direction_with_maximum_number_of_cars(direction);
            remove_first_car(direction);
            removed_car = 1;
        }
        else if ((*get_queue(direction)).size() > 0) {
            remove_first_car(direction);
            removed_car = 1;
        }
        else if (get_other_direction_with_maximum_number_of_cars(direction) != 'X') {
            direction = get_other_direction_with_maximum_number_of_cars(direction);
            remove_first_car(direction);
            removed_car = 1;
        }

        update_waiting_times('N', removed_car);
        update_waiting_times('E', removed_car);
        update_waiting_times('S', removed_car);
        update_waiting_times('W', removed_car);

        pthread_mutex_unlock(&access_a_queue_mutex);

        if (removed_car == 1) {
            pthread_sleep(1);
        }

        current_time_police = time(NULL);
    }
    pthread_exit(0);
}

int main(int argc, char *argv[]){
    time_t start_time = time(NULL);
    int i = 1;
    while (i < argc) {
        if (strcmp(argv[i], "-p") == 0) {
            prob = atof(argv[i+1]);
        }
        else if (strcmp(argv[i], "-s") == 0) {
            end_time = start_time + atoi(argv[i+1]);
        }
        i++;
    }

    pthread_mutex_init(&access_a_queue_mutex, NULL);

    pthread_attr_init(&thread_attribute);    

    add_new_car('N');
    add_new_car('E');
    add_new_car('S');
    add_new_car('W');

    printf("Number of cars in direction N in the beginning of the program: %i\n", car_queue_N.size());
    printf("Number of cars in direction E in the beginning of the program: %i\n", car_queue_E.size());
    printf("Number of cars in direction S in the beginning of the program: %i\n", car_queue_S.size());
    printf("Number of cars in direction W in the beginning of the program: %i\n", car_queue_W.size());
    
    long id_N = 0;
    pthread_create(&thread_N, &thread_attribute, car_thread_fn, (void *)id_N);

    long id_E = 1;
    pthread_create(&thread_E, &thread_attribute, car_thread_fn, (void *)id_E);

    long id_S = 2;
    pthread_create(&thread_S, &thread_attribute, car_thread_fn, (void *)id_S);

    long id_W = 3;
    pthread_create(&thread_W, &thread_attribute, car_thread_fn, (void *)id_W);
    
    pthread_create(&thread_police, &thread_attribute, police_thread_fn, NULL); // Causes segmentation fault. Should be debugged
    
    pthread_join(thread_N, NULL);
    pthread_join(thread_E, NULL);
    pthread_join(thread_S, NULL);
    pthread_join(thread_W, NULL);
    pthread_join(thread_police, NULL); // Causes segmentation fault. Should be debugged.

    printf("Number of cars in direction N at the end of the program: %i\n", car_queue_N.size());
    printf("Number of cars in direction E at the end of the program: %i\n", car_queue_E.size());
    printf("Number of cars in direction S at the end of the program: %i\n", car_queue_S.size());
    printf("Number of cars in direction W at the end of the program: %i\n", car_queue_W.size());

    // car_log_fn(); // Not implemented yet
    // police_log_fn(); // Not implemented yet

    pthread_attr_destroy(&thread_attribute);
    pthread_mutex_destroy(&access_a_queue_mutex);
    pthread_exit(NULL);
    return 0;
}