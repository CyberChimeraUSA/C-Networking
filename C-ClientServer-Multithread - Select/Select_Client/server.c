#include <stdio.h>
#include <string.h>    //strlen
#include <stdlib.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <pthread.h> //for threading , link with lpthread
#include <unistd.h>

//declare server handle
void *server_handler (void *fd_pointer);

int main()
{
    int listenfd, connfd, *new_sock;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr; 
   
   listenfd = socket(AF_INET,SOCK_STREAM,0);
   if (listenfd == -1)
   {
	  perror("Could not create Socket \n"); 
   }
	puts("Socket Created");
  
  
   bzero(&servaddr,sizeof (servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr = INADDR_ANY;
   servaddr.sin_port = htons(8888);
   
   if (bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0)
   {
	   perror("bind failed");
	   return 1;
   }
   puts("bind success");
   listen(listenfd, 5);
   

   puts("Waiting for connections");
   clilen = sizeof(cliaddr);
   while ((connfd = accept(listenfd,(struct sockaddr *)&cliaddr,&clilen)))
  
	{
		
		puts("Connection accepted");
	
		pthread_t server_thread;
        new_sock = malloc(1);
        *new_sock = connfd;
		pthread_create(&server_thread,NULL,server_handler,(void*) new_sock);
	}
	if (connfd < 0)
	{
		perror("Accecpt Failed");
		return 1;
	}
    
	return 0;
   
   
   
	
}

//server thread handler
void *server_handler (void *fd_pointer)
{
	
	printf("Hello Server Handler \n");
	int sock = *(int *)fd_pointer;
    while (1)
	{
	  sleep(2);
	  send(*(int *)fd_pointer, "*Hell0 world!*\n", 15, 0);
	  	
	}
  
    free(fd_pointer);
     
    return 0;
	
}

