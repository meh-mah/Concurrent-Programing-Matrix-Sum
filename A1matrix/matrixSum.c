
/* calculating matrix sum, min, max, and corresponding position in matrix using pthreads

features: uses a bag of task; the main thread prints the final results.

usage under Linux:
gcc matrixSum.c -lpthread
a.out size numWorkers

*/

#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#define MAXSIZE 10000 //maximum matrix size 
#define MAXWORKERS 10 // maximum number of workers

//this is the lock used for updating variables
pthread_mutex_t lock_update_variables; 
// this the lock used for the bag of task
pthread_mutex_t lock_bag;
// condition variable for leaving
pthread_cond_t go; 

int numWorkers; // number of workers 
int numArrived = 0; // number who have arrived 

// variables to keep final result
int f_sum, f_max, f_maxi, f_maxj, f_min, f_mini, f_minj;
int workers_count;
int row;
double start_time, end_time; // start and end times
int size; 

int matrix[MAXSIZE][MAXSIZE]; // matrix 

void *Worker(void *);

// Used to update partial results (sum min max and corresponding positions.
// here p means partial (psum), i means i position and j means j position
void update(int psum, int pmax, int pmaxi, int pmaxj, int pmin, int pmini, int pminj) {
    // lock on shared variable to prevent racing 
    pthread_mutex_lock(&lock_update_variables);
    //update the sum by adding current value
    f_sum += psum;
    workers_count++;
    
    //update the max
    if (pmax > f_max){
        f_max = pmax;
        f_maxi = pmaxi;
        f_maxj = pmaxj; 
    }
    //update min
    if (pmin < f_min){
        f_min = pmin;
        f_mini = pmini;
        f_minj = pminj;
      }
  
  //if all elements of the matrix is processed awake others
  if (workers_count == size)
    pthread_cond_broadcast(&go);

  pthread_mutex_unlock(&lock_update_variables);
}

// timer 
double read_timer() {
  static bool initialized = false;
  static struct timeval start;
  struct timeval end;
  if( !initialized )
  {
    gettimeofday( &start, NULL );
    initialized = true;
  }
  gettimeofday( &end, NULL );
  return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

// read command line, initialize, and create threads
int main(int argc, char *argv[]) {
  int i, j;
  long l; // use long in case of a 64-bit system 
  pthread_attr_t attr;
  pthread_t workerid[MAXWORKERS];

  // set global thread attributes
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  // initialize mutex and condition variable
  pthread_mutex_init(&lock_update_variables, NULL);
  pthread_mutex_init(&lock_bag, NULL);
  pthread_cond_init(&go, NULL);

  workers_count = 0;
  f_min = 100;
  row = 0;

  // read command line args if any
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;
  
  // Initialize elements of the matrix to random values
  srand ( time(NULL) );
  for (i = 0; i < size; i++) {
         for (j = 0; j < size; j++) {
             //assign random number between 0-100
      matrix[i][j] = rand()%101;
         }
  }

  // print the matrix 
#ifdef DEBUG
  for (i = 0; i < size; i++) {
         printf("[ ");
         for (j = 0; j < size; j++) {
         printf(" %d", matrix[i][j]);
         }
         printf(" ]\n");
  }
#endif

  // do the parallel work: create the workers 
  start_time = read_timer();
  for (l = 0; l < numWorkers; l++)
    pthread_create(&workerid[l], &attr, Worker, (void *) l);
  
  // This causes the thread to release lock and wait on condition
  pthread_cond_wait(&go, &lock_update_variables);
  
  end_time = read_timer();
  
  //print result
  printf("The total is %d\n", f_sum);
  printf("The max value is %d .\n at position ( %d, %d)\n", f_max, f_maxi, f_maxj);
  printf("The min value is %d .\n at position ( %d, %d)\n", f_min, f_mini, f_minj);
  printf("The execution time is %g sec\n", end_time - start_time);

  pthread_exit(NULL);
}
//returns the number of the row to process out of the bag by reading a value of the counter and incrementing the counter 
int getRow() {
  pthread_mutex_lock(&lock_bag);
  
  int current = row;
  row++;
  
  pthread_mutex_unlock(&lock_bag);
  return current;
}

void *Worker(void *arg) {
    int total, i, j;
    int min, mini, minj, max, maxi, maxj;
//get a row from the bag of tasks

  int row = 0;
  while (row < size) {
    
    row = getRow();
  
    if (row >= size)
      break;
    

#ifdef DEBUG
    printf("worker %d (pthread id %d) calcultaion row %d of size: %d\n", pthread_self(), pthread_self(), row, size);
#endif
    
    
//initialize variables
    i = row;
    total = 0;
    max = 0;
    maxi = 0;
    maxj = 0;
    min = 100;
    mini = 0;
    minj = 0;
    
    //process all columns of the assigned row
    for (j = 0; j < size; j++) {
        //check if current value is larger. If yes update the max value and its position
        if (matrix[i][j] > max) {
            max = matrix[i][j];
            maxi = i;
            maxj = j;
        }
        //check if current value is smaller. If yes update the min value and its position 
        if (matrix[i][j] < min) {
        min = matrix[i][j];
        mini = i;
        minj = j;
      }
        //update partial sum
      total += matrix[i][j];
    }
    //send the values to update final result variables
    update(total, max, maxi, maxj, min, mini, minj);
  }
} 