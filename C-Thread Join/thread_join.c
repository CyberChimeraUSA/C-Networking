
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//global thread declaration
pthread_t ntid1,ntid2;


//called from pthread_create
void *thr_fn(void *arg)
{
  printf("in thr_fn, retruning \n");
  return 0;
}
void *thr_fn2(void *arg)
{
  printf("in thr_fn2, exiting \n");
  pthread_exit(0);
}

int main(void)
{
  int err;
  //tret used for pthread_join
  void *tret;
  
  //creates our new thread
  err = pthread_create(&ntid1, NULL, thr_fn, NULL);
  //generic error handeling
  if (err != 0)
  {
	 perror("Error printed by perror");
     exit(EXIT_FAILURE);
  }
  
  //creates our new thread
  err = pthread_create(&ntid2, NULL, thr_fn2, NULL);
  //generic error handeling
  if (err != 0)
  {
	 perror("Error printed by perror");
     exit(EXIT_FAILURE);
  }
  
  //pthread_join
  err = pthread_join(ntid1, &tret);
  //generic error handeling
  if (err != 0)
  {
	 perror("Error printed by perror");
     exit(EXIT_FAILURE);
	  
  }
  
  //pthread_join
  err = pthread_join(ntid2, &tret);
  //generic error handeling
  if (err != 0)
  {
	 perror("Error printed by perror");
     exit(EXIT_FAILURE);
	  
  }
  printf("threads join success \n");
  sleep(1);
  exit(0);
}