#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <iostream>
#include <queue>

using namespace std;

#define NUM_THREADS 5 /* 4 different sections + 1 control center */

struct Train {
  int train_id;
  int length;
  char start_point;
  char end_point;
  time_t arrival_time;
  time_t departure_time;
  bool will_breakdown;
  int breakdown_moment;
  int countdown;
};

time_t current_time;
time_t current_times[5];
time_t start_time;
char time_string[100];
struct tm * timeinfo;
int simulation_time; /* simulation time in second */
int total_trains; /* total number of trains in 4 sectoins */
int seed; /* seed for randomization */
double p; /* probabiltiy to arrive sections A, E or F */
pthread_mutex_t section_mutex;
bool clearance_flag = false;
pthread_barrier_t barr;

queue <Train> trains_in_AC;
queue <Train> trains_in_BC;
queue <Train> trains_in_DE;
queue <Train> trains_in_DF;
queue <Train> trains_in_CD;
queue <Train> waiting_passage_AC;
queue <Train> waiting_passage_BC;
queue <Train> waiting_passage_DE;
queue <Train> waiting_passage_DF;
queue <Train> trains_in_AC_departure;
queue <Train> trains_in_BC_departure;
queue <Train> trains_in_DE_departure;
queue <Train> trains_in_DF_departure;
queue <Train> trains_garbage;

vector<string> controlcenterlogvector; 
char cclog[500][500];


bool is_tunnel_full;

int thread_ids[4] = {0,1,2,3};
char sections[4] = {'A','B','E','F'};

char dest_of_A[2] = {'E', 'F'};
char dest_of_B[2] = {'E', 'F'};
char dest_of_E[2] = {'A', 'B'};
char dest_of_F[2] = {'A', 'B'};

int did_one_second_pass = 0; // this variable is for running center thread after all 4 section threads run once.
pthread_cond_t section_cv; // section condition variable

int train_log_counter = 0;
int cclogline = 0;

int pthread_sleep(int seconds);
bool nextBoolean(double prob);
int countdown;

char get_destination(char departure, int index){
  if(departure == 'A'){
    return dest_of_A[index];
  }
  if(departure == 'B'){
    return dest_of_B[index];
  }
  if(departure == 'E'){
    return dest_of_E[index];
  } 
  return dest_of_F[index];
}

int breakdownmoment(int trainlength){
  int num = 0;
  if(trainlength == 100){
    // will generate 0 or 1
    num = rand() % 2;
  } else {
    //will generate 0, 1 or 2
    num = rand() % 3;
  }
  return num;
}

void *printCurrentTimeThreadFunc(void *unused){
  time_t current_time = time(NULL);
  timeinfo = localtime(&current_time);
  strftime (time_string,100,"Control Center time:  %X.",timeinfo);
  puts (time_string);

}

void printControlCenterLog(){
  char str[30];
  int i;
  int size;
  FILE *cc_log_file;
  cc_log_file = fopen("control-center.log","a+");

  fprintf(cc_log_file, "Event\tEvent Time\tTrain ID\tTrains Waiting Passage\n");
  for(int i = 0;i < 10;i++){
    cout << "logginggg" << endl;
    fprintf(cc_log_file,"%s",cclog[i]);
  }
  fclose(cc_log_file);
}

void printTrainLog(){
  char str[30];
  int i;
  int size;
  FILE *train_log_file;
  train_log_file = fopen("train.log","a+");

  if(train_log_counter == 0){
    
    fprintf(train_log_file, "Train ID\tStarting Point\tDestination Point\tLength(m)\tArrival Time\tDeparture Time\n");
  }

  queue<Train> printed = trains_garbage;
  while(!printed.empty()){
    
    
    char strt[100];
    char endd[100];
    
    struct tm * timeinfo1;
    
    struct tm * timeinfo2;
    time_t current_t1 = printed.front().arrival_time;
    timeinfo1 = localtime(&current_t1);
    strftime(strt,100,"%X.",timeinfo1);
    
    time_t current_t2 = printed.front().departure_time;
    timeinfo2 = localtime(&current_t2);
    strftime (endd,100,"%X.",timeinfo2);
    
    fprintf(train_log_file, "%d\t%c\t%c\t%d\t%s\t%s\n",printed.front().train_id,printed.front().start_point,printed.front().end_point,printed.front().length,strt,endd);

    printed.pop();
    
  }
  fclose(train_log_file);

  
}

void printTrainInfo(struct Train tren){


  char strt[100];
  char endd[100];
  
  struct tm * timeinfo1;
  
  struct tm * timeinfo2;
  time_t current_t1 = tren.arrival_time;
  timeinfo1 = localtime(&current_t1);
  strftime(strt,100,"%X.",timeinfo1);
  
  time_t current_t2 = tren.departure_time;
  timeinfo2 = localtime(&current_t2);
  strftime (endd,100,"%X.",timeinfo2);
	
  cout << tren.start_point << " " << tren.end_point << " " << tren.length <<" "<< strt << " " << endd << " " << tren.train_id << endl;


}


void printQueue(std::queue<Train> q){
  queue<Train> printed = q;

  while(!printed.empty()){
    cout << "Printing Queue";
    printTrainInfo(printed.front());
    printed.pop();
  }

  cout << endl;
  //cout <<"Trains in BC size: " << trains_in_BC.size() << endl;
  cout <<"Waiting Passage BC size: " << waiting_passage_BC.size() << endl;
}


void *write_train_log(void *trn){
  //Train *train = (Train *)trn;
  char str[30];
  int i;
  int size;
  FILE *train_log_file;
  train_log_file = fopen("train.log","a+");

  if(train_log_counter == 0){
    
    fprintf(train_log_file, "Train ID\tStarting Point\tDestination Point\tLength(m)\tArrival Time\tDeparture Time\n");
  }
  size = waiting_passage_AC.size();
  
  for(i = 0; i < size; i++){
    char time_s[100];
    struct tm * timeinf;
    time_t arrival = waiting_passage_AC.front().arrival_time;
    timeinf = localtime(&arrival);
    strftime (time_s,100,"Now it's %X.",timeinf);

    fprintf(train_log_file, "%d\t%c\t%c\t%d\t%s\n",waiting_passage_AC.front().train_id,waiting_passage_AC.front().start_point,waiting_passage_AC.front().end_point,waiting_passage_AC.front().length,time_s);


    waiting_passage_AC.pop();
  }
 
  fclose(train_log_file);
  pthread_exit(0);

}

void printTime(){
    char time_s[100];
    struct tm * timeinf;
    time_t arrival = time(NULL);
    timeinf = localtime(&arrival);
    strftime (time_s,100,"%X.",timeinf);
    puts(time_s);
}

void *section_thread_function(void *section){

  long thread_id = (long)section;
  char departure = sections[thread_id];
  
  current_times[thread_id] = time(NULL);

  while(current_times[thread_id] < start_time + simulation_time){

  
    //timeinfo = localtime(&current_times[thread_id]);
    //strftime (time_string,100,"Now it's %X.",timeinfo);
    //printf("[%c]\n",departure);
    //puts (time_string);

    
    
    //if(total_trains > 10){ /* consider the train in the tunnel later */
    //pthread_sleep(1);
    //}

    pthread_mutex_lock(&section_mutex);
    //cout << thread_id << endl;
    
    if(departure == 'A' || departure == 'E' || departure == 'F'){

      
      if(nextBoolean(p)){ /* generate a train with a probability of p */

	struct Train train;
	train.train_id = total_trains;
	train.start_point = departure;

	//cout << "creation from 3:" << departure << endl;

	if(nextBoolean(0.7)){
	  train.length = 100;
	} else {
	  train.length = 200;
	}

	if(nextBoolean(0.5)){
	  train.end_point = get_destination(departure, 0); /* take first indexes of mapping arrays*/ 
	} else {
	  train.end_point = get_destination(departure, 1); /* take second indexes of mapping arrays*/
	}

	if(nextBoolean(0.1)){
	  train.will_breakdown = true;
	} else{
	  train.will_breakdown = false;
	}

	train.arrival_time = time(NULL);
	if(departure == 'A'){
	  //cout << "size before pushing into AC" << trains_in_AC.size() << endl;
	  trains_in_AC.push(train);
	} else if(departure == 'E'){
	  trains_in_DE.push(train);
	} else { /* departure = 'F' */
	  trains_in_DF.push(train);
	}

	total_trains++;
      }
      
    }
    
    if(departure == 'B'){
      
      if(nextBoolean(1-p)){ /* generate a train with a probability of 1-p */
      
	struct Train train;
	train.train_id = total_trains;
	train.start_point = departure;

	//cout << "creation from 1:" << departure << endl;

	
	if(nextBoolean(0.7)){
	  train.length = 100;
	} else {
	  train.length = 200;
	}

	if(nextBoolean(0.5)){
	  train.end_point = get_destination(departure, 0); /* take first indexes of mapping arrays*/ 
	} else {
	  train.end_point = get_destination(departure, 1); /* take second indexes of mapping arrays*/
	}

	if(nextBoolean(0.1)){
	  train.will_breakdown = true;
	} else {
	  train.will_breakdown = false;
	}

	train.arrival_time = time(NULL);
	trains_in_BC.push(train);
	total_trains++;
      }
    }
    //did_one_second_pass++;

    //if(did_one_second_pass % 4 == 0){ // we make sure that each 4 run at least once
    //cout << "signal out" << endl;
    // pthread_cond_signal(&section_cv); // center thread can now schedule. 
    //}


    pthread_mutex_unlock(&section_mutex);



    //pthread_sleep(1);
    pthread_barrier_wait(&barr);
    sleep(1);
    current_times[thread_id] = time(NULL);
  }

  pthread_exit(0);
}

void *center_thread_function(void *param){

  time_t curr_time;
  int i;
  int index;
  int max;
  int chosen;
  bool inAction = false;
  bool isTunnelFull = false;
  // pthread_t printCurrentTimeThread;
  // pthread_attr_t attr;
  //pthread_attr_init(&attr);
  //pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE); /* for some systems */

  bool flag = true;
  curr_time = time(NULL);
  while(curr_time < start_time + simulation_time){
    
    
    //pthread_mutex_lock(&section_mutex);

    //cout << "cc in work " << endl;
    //pthread_cond_wait(&section_cv, &section_mutex);
    pthread_barrier_wait(&barr);
    // at each second, print current status, changed to the next status.
    //printQueue(waiting_passage_BC);

    cout << "Control Center's Turn ";
    printTime();
    if(total_trains > 10){
      //clearance_flag = true;
    }

    if(clearance_flag){
      
    } else {  
      
      int waiting_sizes[4];

      if(trains_in_AC_departure.size() > 0){

	trains_in_AC_departure.front().departure_time = time(NULL);

	printTrainInfo(trains_in_AC_departure.front());

	trains_in_AC_departure.front().departure_time = time(NULL);
	trains_garbage.push(trains_in_AC_departure.front());
	trains_in_AC_departure.pop();
      }
      if(trains_in_BC_departure.size() > 0){

	trains_in_BC_departure.front().departure_time = time(NULL);
        
	printTrainInfo(trains_in_BC_departure.front());
	
	trains_in_BC_departure.front().departure_time = time(NULL);
	trains_garbage.push(trains_in_BC_departure.front());
	trains_in_BC_departure.pop();
      }
      if(trains_in_DE_departure.size() > 0){
        
	trains_in_DE_departure.front().departure_time = time(NULL);

	printTrainInfo(trains_in_DE_departure.front());
	
	trains_in_DE_departure.front().departure_time = time(NULL);
	trains_garbage.push(trains_in_DE_departure.front());
	trains_in_DE_departure.pop();
      }
      if(trains_in_DF_departure.size() > 0){
       
	trains_in_DF_departure.front().departure_time = time(NULL);

	printTrainInfo(trains_in_DF_departure.front());

	trains_in_DF_departure.front().departure_time = time(NULL);
	trains_garbage.push(trains_in_DF_departure.front());
	trains_in_DF_departure.pop();
      }

      if(trains_in_CD.size() > 0){

	if((countdown == trains_in_CD.front().countdown - trains_in_CD.front().breakdown_moment) && trains_in_CD.front().will_breakdown){
	  
	  char strevent[200];
	  char time_s[100];
	  struct tm * timeinf;
	  time_t breakdownTime = time(NULL);
	  timeinf = localtime(&breakdownTime);
	  strftime (time_s,100,"%X.",timeinf);
	  
	  cout << "Breakdown" << " " << time_s << " " << trains_in_CD.front().train_id << endl;
	  //string var = string("Tunnel Passing ") + time_s + string(" ") + trainToBePopped.train_id;
	  //controlcenterlogvector.push_back(var);
	  sprintf(strevent,"Breakdown %s %d\n",time_s,trains_in_CD.front().train_id);

	    
	  for(int i = 0; i < 100; i++){
	    cclog[cclogline][i] = strevent[i];
	  }
	  
	  cclogline++;
	}
	
	countdown--;
	if(countdown == 0){
	  cout << "popping" << endl;


	  if(trains_in_CD.front().end_point == 'A'){
	    trains_in_AC_departure.push(trains_in_CD.front());
	  } else if(trains_in_CD.front().end_point == 'B'){
	    trains_in_BC_departure.push(trains_in_CD.front());
	  } if(trains_in_CD.front().end_point == 'E'){
	    trains_in_DE_departure.push(trains_in_CD.front());
	  } if(trains_in_CD.front().end_point == 'F'){
	    trains_in_DF_departure.push(trains_in_CD.front());
	  }
	  
	  //cout << trains_in_CD.front().start_point << " " << trains_in_CD.front().end_point << strt << " " << endd << endl;
	  trains_in_CD.pop();


	  //alt kısım tam yeni trenin girme anı



	  waiting_sizes[0] = waiting_passage_AC.size();
	  waiting_sizes[1] = waiting_passage_BC.size();
	  waiting_sizes[2] = waiting_passage_DE.size();
	  waiting_sizes[3] = waiting_passage_DF.size();
	  
	  max = waiting_sizes[0];
	  index = 0; 
	  for(i = 0; i < 4; i++){
	    if(waiting_sizes[i] > max){
	      index = i;
	      max = waiting_sizes[i];
	    }
	  }
	  
	  struct Train trainToBePopped;
	  
	  if(max !=  0){ // there is at least one train waiting passage
	    cout << "max buldu" << endl;
	    if(index == 0){ // priority is in AC
	      trainToBePopped =  waiting_passage_AC.front();
	      waiting_passage_AC.pop();
	      
	    } else if(index == 1){ // priority is in BC
	      trainToBePopped =  waiting_passage_BC.front();
	      waiting_passage_BC.pop();
	      
	    } else if(index == 2){ // priority is in DE
	      trainToBePopped =  waiting_passage_DE.front();
	      waiting_passage_DE.pop();
	      
	    } else { // index = 3 and priority is in DF
	      trainToBePopped =  waiting_passage_DF.front();
	      waiting_passage_DF.pop();
	      
	    }
	    inAction = true;
	    printTrainInfo(trainToBePopped);
	    trainToBePopped.breakdown_moment = breakdownmoment(trainToBePopped.length);
	    char strevent[300];
	    char waitingqueue[500];
	    char time_s[100];
	    char num[50];
	    struct tm * timeinf;
	    time_t arrival = time(NULL);
	    timeinf = localtime(&arrival);
	    strftime (time_s,100,"%X.",timeinf);
	    
	    cout << "Tunnel Passing" << " " << time_s << " " << trainToBePopped.train_id << endl;
	    //string var = string("Tunnel Passing ") + time_s + string(" ") + trainToBePopped.train_id;
	    //controlcenterlogvector.push_back(var);
	    int l = waiting_passage_AC.size();
	    queue<Train> pr0 = waiting_passage_AC;
	    strcpy(waitingqueue," ");
	    for(int i = 0; i < l; i++){
	      sprintf(num,"%d",pr0.front().train_id);
	      //cout << i << endl;
	      strcat(waitingqueue,num);
	      strcat(waitingqueue," ");
	      pr0.pop();
	    }

	    l = waiting_passage_BC.size();
	    queue<Train> pr1 = waiting_passage_BC;
	    for(int i = 0; i < l; i++){
	      sprintf(num,"%d",pr1.front().train_id);
	      //cout << i << endl;
	      strcat(waitingqueue,num);
	      strcat(waitingqueue," ");
	      pr1.pop();
	    }

	    l = waiting_passage_DE.size();
	    queue<Train> pr2 = waiting_passage_DE;
	    for(int i = 0; i < l; i++){
	      sprintf(num,"%d",pr2.front().train_id);
	      //cout << i << endl;
	      strcat(waitingqueue,num);
	      strcat(waitingqueue," ");
	      pr2.pop();
	    }

	    l = waiting_passage_DF.size();
	    queue<Train> pr3 = waiting_passage_DF;
	    for(int i = 0; i < l; i++){
	      sprintf(num,"%d",pr3.front().train_id);
	      //cout << i << endl;
	      strcat(waitingqueue,num);
	      strcat(waitingqueue," ");
	      pr3.pop();
	    }
	    
	    sprintf(strevent,"Tunnel Passing %s\t%d\t%s\n",time_s,trainToBePopped.train_id,waitingqueue);

	    
	    for(int i = 0; i < 100; i++){
	      cclog[cclogline][i] = strevent[i];
	    }

	    cclogline++;
	    
	    trains_in_CD.push(trainToBePopped);




	    if(trains_in_CD.front().length == 100){
	      countdown = 2;
	      if(trains_in_CD.front().will_breakdown){
		countdown += 4;
	      }
	      trains_in_CD.front().countdown = countdown;
	    }else {
	      countdown = 3;
	      if(trains_in_CD.front().will_breakdown){
		countdown += 4;
	      }
	      trains_in_CD.front().countdown = countdown;
	    }
	    
	 
	  } else {
	    // no trains to pass the tunnel
  
	  }   
	    
	    




	  //ust kısım tam yeni trenin girme anı
	}
      } else {
	// tunelde tren yoksa
	waiting_sizes[0] = waiting_passage_AC.size();
	waiting_sizes[1] = waiting_passage_BC.size();
	waiting_sizes[2] = waiting_passage_DE.size();
	waiting_sizes[3] = waiting_passage_DF.size();

	max = waiting_sizes[0];
	index = 0; 
	for(i = 0; i < 4; i++){
	  if(waiting_sizes[i] > max){
	    index = i;
	    max = waiting_sizes[i];
	  }
	}
	
	struct Train trainToBePopped;

	if(max !=  0){ // there is at least one train waiting passage
	  cout << "max buldu" << endl;
	  if(index == 0){ // priority is in AC
	    trainToBePopped =  waiting_passage_AC.front();
	    waiting_passage_AC.pop();
	    
	  } else if(index == 1){ // priority is in BC
	    trainToBePopped =  waiting_passage_BC.front();
	    waiting_passage_BC.pop();
	    
	  } else if(index == 2){ // priority is in DE
	    trainToBePopped =  waiting_passage_DE.front();
	    waiting_passage_DE.pop();
	  
	  } else { // index = 3 and priority is in DF
	    trainToBePopped =  waiting_passage_DF.front();
	    waiting_passage_DF.pop();
	  
	  }
	  inAction = true;
	  printTrainInfo(trainToBePopped);
	  trainToBePopped.breakdown_moment = breakdownmoment(trainToBePopped.length);
	    
	  char strevent[300];
	  char time_s[100];
	  char waitingqueue[500];
	  char num[50];
	  struct tm * timeinf;
	  time_t arrival = time(NULL);
	  timeinf = localtime(&arrival);
	  strftime (time_s,100,"%X.",timeinf);
	  
	  cout << "Tunnel Passing" << " " << time_s << " " << trainToBePopped.train_id << endl;
	  //string var = string("Tunnel Passing ") + time_s + string(" ") + trainToBePopped.train_id;
	  //controlcenterlogvector.push_back(var);
	  int l = waiting_passage_AC.size();
	  queue<Train> pr0 = waiting_passage_AC;
	  strcpy(waitingqueue," ");
	  for(int i = 0; i < l; i++){
	    //cout << i << endl;
	    sprintf(num,"%d",pr0.front().train_id);
	    strcat(waitingqueue,num);
	    strcat(waitingqueue," ");
	    pr0.pop();
	  }
	  
	  l = waiting_passage_BC.size();
	  queue<Train> pr1 = waiting_passage_BC;
	  for(int i = 0; i < l; i++){
	    //cout << i << endl;
	    sprintf(num,"%d",pr1.front().train_id);
	    strcat(waitingqueue,num);
	    strcat(waitingqueue," ");
	    pr1.pop();
	  }
	  
	  l = waiting_passage_DE.size();
	  queue<Train> pr2 = waiting_passage_DE;
	  for(int i = 0; i < l; i++){
	    sprintf(num,"%d",pr2.front().train_id);
	    //cout << i << endl;
	    strcat(waitingqueue,num);
	    strcat(waitingqueue," ");
	    pr2.pop();
	  }
	  
	  l = waiting_passage_DF.size();
	  queue<Train> pr3 = waiting_passage_DF;
	  for(int i = 0; i < l; i++){
	    sprintf(num,"%d",pr3.front().train_id);
	    //cout << i << endl;
	    strcat(waitingqueue,num);
	    strcat(waitingqueue," ");
	    pr3.pop();
	  }
	  
	  sprintf(strevent,"Tunnel Passing %s\t%d\t%s\n",time_s,trainToBePopped.train_id,waitingqueue);
	  
	  
	  
	  
	  for(int i = 0; i < 100; i++){
	    cclog[cclogline][i] = strevent[i];
	  }
	  cclogline++;
	  
	  trains_in_CD.push(trainToBePopped);
	    if(trains_in_CD.front().length == 100){
	      countdown = 2;
	      if(trains_in_CD.front().will_breakdown){
		countdown += 4;
	      }
	      trains_in_CD.front().countdown = countdown;
	    }else {
	      countdown = 3;
	      if(trains_in_CD.front().will_breakdown){
		countdown += 4;
	      }
	      trains_in_CD.front().countdown = countdown;
	    }
      	
	  //cout << "here and" << endl;
	  //pthread_create(&printCurrentTimeThread, &attr, printCurrentTimeThreadFunc, NULL);
	  //pthread_join(printCurrentTimeThread, NULL);
	  //cout << "there" << endl;
	
	} else {
	  // no trains to pass the tunnel
  
	}
      }
    }//clearance flag







    
      
	
      
    if(trains_in_AC.size() > 0){
      waiting_passage_AC.push(trains_in_AC.front());
      trains_in_AC.pop();
    }
    if(trains_in_BC.size() > 0){
      waiting_passage_BC.push(trains_in_BC.front());
      trains_in_BC.pop();
    }
    if(trains_in_DE.size() > 0){
      waiting_passage_DE.push(trains_in_DE.front());
      trains_in_DE.pop();
    }
    if(trains_in_DF.size() > 0){
      waiting_passage_AC.push(trains_in_DF.front());
      trains_in_DF.pop();
    }
    
    //cout << "unlocking control center thread" << endl;
    //pthread_mutex_unlock(&section_mutex);
    sleep(1);
    curr_time = time(NULL);
  }//while

  pthread_exit(0);
  
}

int main(int argc, char *argv[]){

  pthread_t thread_AC;
  pthread_t thread_BC;
  pthread_t thread_DE;
  pthread_t thread_DF;

  pthread_t thread_center;

  pthread_t train_log_thread;

  struct Train deneme;
  
  long t1 = 0, t2 = 1, t3 = 2, t4 = 3;

  pthread_attr_t attr;
  
  int i;
  
  for(i = 1; i < argc; i++) { /* argv[0] is the program name */

    if(strcmp(argv[i], "-s") == 0) {

      simulation_time = atoi(argv[i+1]);

    } else if(strcmp(argv[i], "-p") == 0) {

      p = (double) atof(argv[i+1]);

    } else if(strcmp(argv[i], "-r") == 0){

      seed = atoi(argv[i+1]);
      
    }
    
  }

  printf("simulation time: %d\n",simulation_time);
  printf("probability: %f\n",p);
  printf("seed number: %d\n",seed);
 
  pthread_mutex_init(&section_mutex, NULL);
  //pthread_cond_init(&section_cv, NULL);
  pthread_barrier_init(&barr,NULL,5);
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE); /* for some systems */
  
  srand(seed);

  total_trains = 0;


  start_time = time(NULL);

    
  current_time = time(NULL);
  timeinfo = localtime(&current_time);
  strftime (time_string,100,"Now it's %X.",timeinfo);
  printf("%s\n",time_string);


  
  pthread_create(&thread_AC, &attr, section_thread_function, (void *)t1);
  pthread_create(&thread_BC, &attr, section_thread_function, (void *)t2);
  pthread_create(&thread_DE, &attr, section_thread_function, (void *)t3);
  pthread_create(&thread_DF, &attr, section_thread_function, (void *)t4);
  pthread_create(&thread_center, &attr, center_thread_function, NULL);  

  
  pthread_join(thread_AC, NULL);
  pthread_join(thread_BC, NULL);
  pthread_join(thread_DE, NULL);
  pthread_join(thread_DF, NULL);
  cout << "will join thread center" << endl;
  pthread_join(thread_center, NULL);

  printQueue(trains_garbage);
  printTrainLog();
  printControlCenterLog();
  //pthread_create(&train_log_thread, &attr, write_train_log, (void *)&deneme);
  //pthread_join(train_log_thread,NULL);
  
  //cout << "Departure\tDestination\tLength\tWill breakdown?" << endl;
    
  /* for(i = 0; i < 100; i++){

    int section = (rand() % 4) + 1;
    if(section == 1){
      section_thread_function("A");
    }else if(section == 2){
      section_thread_function("B");
    }else if(section == 3){
      section_thread_function("E");
    }else{
      section_thread_function("F");  
      }*/

    //if(trains.size() != 0){
    // cout << trains.front().start_point << "\t" << trains.front().end_point << "\t"<< trains.front().length << "\t" << trains.front().will_breakdown << endl;
    // trains.pop();
    //}
   
  //}


  pthread_attr_destroy(&attr);
  pthread_mutex_destroy(&section_mutex);
  //cout <<"mutex destroyed" << endl;
  //pthread_cond_destroy(&section_cv);
  pthread_barrier_destroy(&barr);
  //cout <<"barr destroyed"<<endl;
  pthread_exit(NULL);
  // cout << "will finish" <<endl;
  return 0;
}



bool nextBoolean(double probability){
  /*srand(time(NULL));*/
  /*srand(seed);*/
  if((rand() / (double)RAND_MAX) < probability){
    return true;
  }
  return false;
}





/******************************************************************************
  pthread_sleep takes an integer number of seconds to pause the current thread
  original by Yingwu Zhu
  updated by Muhammed Nufail Farooqi
*****************************************************************************/

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
  int res =  pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
  pthread_mutex_unlock (&mutex);
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&conditionvar);

  //Upon successful completion, a value of zero shall be returned
  return res;

}

