#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
 
//array multiple threads
pthread_t tid[2];
int counter;
//create mutex lock
pthread_mutex_t lock;
 
void* mutex_fxn(void *arg)
{
	//lock mutex
    pthread_mutex_lock(&lock);
 
    unsigned long i;
    counter += 1;
    printf("\n Job %d has started\n", counter);
 
    for(i=0; i<(0x0000FFFF);i++);
 
    printf("\n Job %d has finished\n", counter);
 
	//unlock mutex
    pthread_mutex_unlock(&lock);
 
}
 
int main(void)
{
    int i = 0;
    int err;
 
    //must init mutex lock
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init has failed\n");
        return 1;
    }
    //create 2 threads
    while(i < 2)
    {
	printf("Thread Created \n");
     err = pthread_create(&(tid[i]), NULL, &mutex_fxn, NULL);
     if (err != 0)
     {
	     perror("Error printed by perror");
         exit(EXIT_FAILURE);
     }
	  
     i++;
    }
 
    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
	//destroy mutex
    pthread_mutex_destroy(&lock);
 
    return 0;
}