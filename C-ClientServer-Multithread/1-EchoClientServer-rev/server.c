#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread


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
  
   //close(connfd);
}

void *server_handler (void *fd_pointer)
{
	printf("Hello Server Handler \n");
	int sock = *(int *)fd_pointer;
    //char client_message[2000];
    int read_size, write_size;
    char *message;
	static char client_message[2000];
    message = " \nHello Server Handler \n";
	//message2 = "Test Send \n";


	static int send_once = 0;
	if (send_once < 1)
	{
	//write(sock,message,strlen(message));
	send_once++;
	}

    while((read_size = recv(sock,client_message,2000,0)) > 0)
   {
     printf("Read Size %d \n", read_size);
     write(sock,client_message,strlen(client_message));
   }
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
	else if(read_size == -1)
    {
        perror("recv failed");
    }
    free(fd_pointer);
     
    return 0;
}
