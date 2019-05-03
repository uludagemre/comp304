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
using namespace std;

struct Car
{
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

queue<Car> car_queue_N;
queue<Car> car_queue_E;
queue<Car> car_queue_S;
queue<Car> car_queue_W;

char id_direction_mapping[4] = {'N', 'E', 'S', 'W'};

float prob;
time_t specified_time;
time_t start_time;
int num_cars = 0;
int police_gone_lazy = 0;

void *create_car_log(void)
{
    ofstream car_log_file("car.log");
    if (!car_log_file.is_open())
    {
        cout << "Unable to open the file" << endl;
    }
    else
    {
        char titlebar[250];
        sprintf(titlebar, "%-20s%-20s%-20s%-20s%-20s%-20s%-20s%-20s%-20s%-20s", "Action", "CarID", "Direction", "Arrival-Time", "Cross-Time", "Wait-Time", "N", "E", "S", "W");
        car_log_file << titlebar << endl;
        car_log_file << "------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------" << endl;
        car_log_file.close();
    }
}

void *create_police_log(void)
{
    ofstream police_log_file("police.log");
    if (!police_log_file.is_open())
    {
        cout << "Unable to open the file" << endl;
    }
    else
    {
        police_log_file << "Time\t\tEvent" << endl;
        police_log_file << "---------------------" << endl;
        police_log_file.close();
    }
}
void *create_position_log(void)
{
    ofstream position_log_file("snapshot.log");
    if (!position_log_file.is_open())
    {
        cout << "Unable to open the file" << endl;
    }
    else
    {
        position_log_file << "    Snapshots   " << endl;
        position_log_file << "------------------\n"
                          << endl;
        position_log_file.close();
    }
}

void *appendPoliceLog(const char *stringToAppend)
{
    std::ofstream outfile;
    outfile.open("police.log", std::ios_base::app);
    outfile << stringToAppend << endl;
}
void *appendPositionLog(int snapShot[])
{
    char currentTime[30];
    time_t c_time = time(0);
    strftime(currentTime, 20, "%H:%M:%S", localtime(&c_time));
    char output[40];
    sprintf(output, "At: %s\n------------------\n", currentTime);
    std::ofstream outfile;
    outfile.open("snapshot.log", std::ios_base::app);
    char positions[40];
    sprintf(positions, "\t%d\t\n\n%d\t\t%d\n\n\t%d\t\n", snapShot[0], snapShot[3], snapShot[1], snapShot[2]);
    outfile << output << endl;
    outfile << positions << endl;
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

queue<Car> *get_queue(char direction)
{
    if (direction == 'N')
    {
        return &car_queue_N;
    }
    else if (direction == 'E')
    {
        return &car_queue_E;
    }
    else if (direction == 'S')
    {
        return &car_queue_S;
    }
    else if (direction == 'W')
    {
        return &car_queue_W;
    }
}

void *appendCarLog(struct Car car_to_be_removed, char *type)
{
    char arrival_time[20];
    strftime(arrival_time, 20, "%H:%M:%S", localtime(&car_to_be_removed.arrival_time));

    char cross_time[20];
    strftime(cross_time, 20, "%H:%M:%S", localtime(&car_to_be_removed.cross_time));

    char outputString[300];
    sprintf(outputString,
            "%-20s%-20d%-20c%-20s%-20s%-20.1f%-20d%-20d%-20d%-20d",
            type,
            car_to_be_removed.id,
            car_to_be_removed.incoming_direction,
            arrival_time,
            cross_time,
            car_to_be_removed.waiting_time,
            (*(get_queue('N'))).size(),
            (*(get_queue('E'))).size(),
            (*(get_queue('S'))).size(),
            (*(get_queue('W'))).size());
    std::ofstream outfile;
    outfile.open("car.log", std::ios_base::app);
    outfile << outputString << endl;
}

time_t *get_time(char direction)
{
    if (direction == 'N')
    {
        return &current_time_N;
    }
    else if (direction == 'E')
    {
        return &current_time_E;
    }
    else if (direction == 'S')
    {
        return &current_time_S;
    }
    else if (direction == 'W')
    {
        return &current_time_W;
    }
}

void add_new_car(char direction)
{
    struct Car new_car;
    new_car.id = num_cars;
    new_car.incoming_direction = direction;
    new_car.arrival_time = time(0);
    new_car.cross_time = time(0);
    new_car.waiting_time = 0.0;
    queue<Car> *car_queue = get_queue(direction);
    (*car_queue).push(new_car);
    num_cars = num_cars + 1;
    // appendCarLog(new_car, "added");
}

void remove_first_car(char direction)
{
    queue<Car> *car_queue = get_queue(direction);
    struct Car car_to_be_removed = (*car_queue).front();
    car_to_be_removed.cross_time = time(0);
    (*car_queue).pop();
    appendCarLog(car_to_be_removed, "removed");
}

void update_waiting_times(char direction, int seconds_passed)
{
    queue<Car> *queue_car = get_queue(direction);
    int queue_size = (*queue_car).size();
    queue<Car> result;
    for (int i = 0; i < queue_size; i++)
    {
        struct Car current_car = (*queue_car).front();
        current_car.waiting_time = current_car.waiting_time + (float)seconds_passed;
        (*queue_car).pop();
        result.push(current_car);
    }
    *queue_car = result;
}

char get_other_direction_with_a_queue_more_than_5_cars(char direction)
{
    queue<Car> *queue_N = get_queue('N');
    queue<Car> *queue_E = get_queue('E');
    queue<Car> *queue_S = get_queue('S');
    queue<Car> *queue_W = get_queue('W');

    int length_of_longest_queue = 0;
    char direction_of_longest_queue = 'X';

    if ((direction != 'N') && ((*queue_N).size() > length_of_longest_queue))
    {
        length_of_longest_queue = (*queue_N).size();
        direction_of_longest_queue = 'N';
    }
    if ((direction != 'E') && ((*queue_E).size() > length_of_longest_queue))
    {
        length_of_longest_queue = (*queue_E).size();
        direction_of_longest_queue = 'E';
    }
    if ((direction != 'S') && ((*queue_S).size() > length_of_longest_queue))
    {
        length_of_longest_queue = (*queue_S).size();
        direction_of_longest_queue = 'S';
    }
    if ((direction != 'W') && ((*queue_W).size() > length_of_longest_queue))
    {
        length_of_longest_queue = (*queue_W).size();
        direction_of_longest_queue = 'W';
    }
    if (length_of_longest_queue >= 5)
    {
        return direction_of_longest_queue;
    }
    return 'X';
}

char get_other_direction_with_maximum_number_of_cars(char direction)
{
    queue<Car> *queue_N = get_queue('N');
    queue<Car> *queue_E = get_queue('E');
    queue<Car> *queue_S = get_queue('S');
    queue<Car> *queue_W = get_queue('W');

    int length_of_longest_queue = 0;
    char direction_of_longest_queue = 'X';

    if ((direction != 'N') && ((*queue_N).size() > length_of_longest_queue))
    {
        length_of_longest_queue = (*queue_N).size();
        direction_of_longest_queue = 'N';
    }
    if ((direction != 'E') && ((*queue_E).size() > length_of_longest_queue))
    {
        length_of_longest_queue = (*queue_E).size();
        direction_of_longest_queue = 'E';
    }
    if ((direction != 'S') && ((*queue_S).size() > length_of_longest_queue))
    {
        length_of_longest_queue = (*queue_S).size();
        direction_of_longest_queue = 'S';
    }
    if ((direction != 'W') && ((*queue_W).size() > length_of_longest_queue))
    {
        length_of_longest_queue = (*queue_W).size();
        direction_of_longest_queue = 'W';
    }
    if (length_of_longest_queue == 0)
    {
        return 'X';
    }
    return direction_of_longest_queue;
}

char get_other_direction_with_a_car_waiting_for_more_than_20_seconds(char direction)
{
    queue<Car> *queue_N = get_queue('N');
    queue<Car> *queue_E = get_queue('E');
    queue<Car> *queue_S = get_queue('S');
    queue<Car> *queue_W = get_queue('W');

    int length_of_longest_waiting = 0;
    char direction_of_queue_with_longest_waiting = 'X';

    if ((direction != 'N') && ((*queue_N).size() > 0) && ((*queue_N).front().waiting_time > length_of_longest_waiting))
    {
        length_of_longest_waiting = ((*queue_N).front()).waiting_time;
        direction_of_queue_with_longest_waiting = 'N';
    }
    if ((direction != 'E') && ((*queue_E).size() > 0) && ((*queue_E).front().waiting_time > length_of_longest_waiting))
    {
        length_of_longest_waiting = ((*queue_E).front()).waiting_time;
        direction_of_queue_with_longest_waiting = 'E';
    }
    if ((direction != 'S') && ((*queue_S).size() > 0) && ((*queue_S).front().waiting_time > length_of_longest_waiting))
    {
        length_of_longest_waiting = ((*queue_S).front()).waiting_time;
        direction_of_queue_with_longest_waiting = 'S';
    }
    if ((direction != 'W') && ((*queue_W).size() > 0) && ((*queue_W).front().waiting_time > length_of_longest_waiting))
    {
        length_of_longest_waiting = ((*queue_W).front()).waiting_time;
        direction_of_queue_with_longest_waiting = 'W';
    }
    if (length_of_longest_waiting > 20)
    {
        return direction_of_queue_with_longest_waiting;
    }
    return 'X';
}

void *car_thread_fn(void *direction_star)
{
    char direction = id_direction_mapping[(long)direction_star];
    int is_n_trap = 0;

    time_t *current_thread_time = get_time(direction);
    *current_thread_time = time(0);

    while (*current_thread_time < end_time)
    {
        pthread_mutex_lock(&access_a_queue_mutex);

        if ((direction == 'E') || (direction == 'S') || (direction == 'W'))
        {
            if ((rand() / (float)RAND_MAX) < prob)
            {
                add_new_car(direction);
            }
        }
        else if (direction == 'N')
        {
            if ((rand() / (float)RAND_MAX) < 1.0 - prob)
            {
                add_new_car(direction);
            }
            else
            {
                is_n_trap = 1;
            }
        }
        pthread_mutex_unlock(&access_a_queue_mutex);
        if (is_n_trap == 1)
        {
            pthread_sleep(20); // Change to 20 after debugging.
        }
        else
        {
            pthread_sleep(1);
        }

        time_t *current_thread_time = get_time(direction);
        *current_thread_time = time(0);
    }
    pthread_exit(0);
}

void *police_thread_fn(void *sth)
{
    current_time_police = time(0);
    char direction = get_other_direction_with_maximum_number_of_cars('X');
    char current_direction;
    int removed_car;
    int police_playing_cell_phone;
    int playing_cell_phone_detected = 0;
    int lines_are_empty = 0;
    int car_hooked = 0;
    int hook_counter = 0;
    time_t h_time;
    pthread_cond_t conditionvarForPolice;
    int conditionVaribaleForPolice = 0;
    int blockTheWay = 0;
    while (current_time_police < end_time)
    {
        pthread_mutex_lock(&access_a_queue_mutex);
        removed_car = 0;
        if ((specified_time == current_time_police) || (specified_time + 1 == current_time_police) || (specified_time + 2 == current_time_police))
        {
            int snapShot[] = {(*(get_queue('N'))).size(),
                              (*(get_queue('E'))).size(),
                              (*(get_queue('S'))).size(),
                              (*(get_queue('W'))).size()};
            appendPositionLog(snapShot);
        }

        lines_are_empty = ((*(get_queue('N'))).size() == 0) && ((*(get_queue('E'))).size() == 0) && ((*(get_queue('S'))).size() == 0) && ((*(get_queue('W'))).size() == 0);
        police_gone_lazy = (current_time_police > start_time + 2) && lines_are_empty;

        if (police_gone_lazy)
        {
            police_playing_cell_phone++;
        }
        if (police_playing_cell_phone == 1)
        {   
            char currentTimeCellPhone[30];
            time_t c_time = time(0)-1;
            strftime(currentTimeCellPhone, 20, "%H:%M:%S", localtime(&c_time));
            strcat(currentTimeCellPhone,"   Police started playing cell phone!");
            appendPoliceLog(currentTimeCellPhone);
            blockTheWay = 1;
            
        }
        conditionVaribaleForPolice = !lines_are_empty && (police_playing_cell_phone > 1 );
        if(conditionVaribaleForPolice){
           
            police_playing_cell_phone = 0;
            char honkTime[30];
            h_time = time(0)-1;
            strftime(honkTime, 20, "%H:%M:%S", localtime(&h_time));
            strcat(honkTime,"   Driver Honked!");
            appendPoliceLog(honkTime);  

        } 
        if((time(0)-1) == (h_time + 3)) {
            blockTheWay =0;
            h_time = -1;
        }
if(!blockTheWay) {

        if (get_other_direction_with_a_car_waiting_for_more_than_20_seconds(direction) != 'X')
        {
            direction = get_other_direction_with_a_car_waiting_for_more_than_20_seconds(direction);
            remove_first_car(direction);
            removed_car = 1;
        }
        else if (get_other_direction_with_a_queue_more_than_5_cars(direction) != 'X')
        {
            direction = get_other_direction_with_a_queue_more_than_5_cars(direction);
            remove_first_car(direction);
            removed_car = 1;
        }
        else if ((*get_queue(direction)).size() > 0)
        {
            remove_first_car(direction);
            removed_car = 1;
        }
        else if (get_other_direction_with_maximum_number_of_cars(direction) != 'X')
        {
            direction = get_other_direction_with_maximum_number_of_cars(direction);
            remove_first_car(direction);
            removed_car = 1;
        }
        
}

        update_waiting_times('N', removed_car);
        update_waiting_times('E', removed_car);
        update_waiting_times('S', removed_car);
        update_waiting_times('W', removed_car);

        pthread_mutex_unlock(&access_a_queue_mutex);

        if (removed_car == 1)
        {
            pthread_sleep(1);
        }

        current_time_police = time(0);
    }
    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    create_car_log();
    create_position_log();
    create_police_log();
    start_time = time(0);
    int i = 1;
    while (i < argc)
    {
        if (strcmp(argv[i], "-p") == 0)
        {
            prob = atof(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-s") == 0)
        {
            end_time = start_time + atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-t") == 0)
        {
            specified_time = start_time + atoi(argv[i + 1]);
        }
        i++;
    }

    pthread_mutex_init(&access_a_queue_mutex, NULL);

    pthread_attr_init(&thread_attribute);

    add_new_car('N');
    add_new_car('E');
    add_new_car('S');
    add_new_car('W');

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

    // car_log_fn(); // Not implemented yet
    // police_log_fn(); // Not implemented yet

    pthread_attr_destroy(&thread_attribute);
    pthread_mutex_destroy(&access_a_queue_mutex);
    pthread_exit(NULL);
    return 0;
}