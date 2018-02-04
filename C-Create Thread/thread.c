
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//global thread declaration
pthread_t ntid;

void threadinfo(const char *s)
{
  pid_t pid;
  pthread_t tid;
  pid = getpid();
  tid = pthread_self();
  //process id will be the same
  printf("%s pid %lu tid %lu (0x%lx)\n", s, (unsigned long)pid, (unsigned long)tid, (unsigned long)tid);
}

//called from pthread_create
void *thr_fn(void *arg)
{
  threadinfo("Info: new thread: ");
  return 0;
}

int main(void)
{
  int err;
  //creates our new thread
  err = pthread_create(&ntid, NULL, thr_fn, NULL);
  //generic error handeling
  if (err != 0)
  {
	 perror("Error printed by perror");
     exit(EXIT_FAILURE);
  }

  threadinfo("Info: main thread:");
  sleep(1);
  exit(0);
}