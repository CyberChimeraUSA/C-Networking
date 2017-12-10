#include<stdio.h> //printf
#include<string.h>    //strlen
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include <errno.h>
#include <unistd.h> 


int main(int argc, char **argv)
{
     int sockfd,sockfd1,n,rv;
    fd_set readfds;
    struct timeval tv;
    char buf1[256], buf2[256];
  
   struct sockaddr_in servaddr; 
   

  
   sockfd = socket(AF_INET,SOCK_STREAM,0);
   if (sockfd == -1)
    {
        perror("Could not create socket");
    }
	
   sockfd1 = socket(AF_INET,SOCK_STREAM,0);
   if (sockfd1 == -1)
    {
        perror("Could not create socket");
    }
   printf("Created Socket \n");
   bzero(&servaddr,sizeof (servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_port = htons(8888);
   servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  
   connect(sockfd, (struct sockaddr *)&servaddr,sizeof(servaddr));
   connect(sockfd1, (struct sockaddr *)&servaddr,sizeof(servaddr));

   
 

while(1)
{
	
FD_ZERO(&readfds);
FD_SET(sockfd, &readfds);
FD_SET(sockfd1, &readfds);


n = sockfd1 + 1;

tv.tv_sec = 7;
tv.tv_usec = 500000;
//select fxn
rv = select(n, &readfds, NULL, NULL, &tv);
printf("After Select: %d \n", rv);
if (rv == -1) {
    perror("select"); 
} else if (rv == 0) {
    printf("Timeout occurred!  No data after 10.5 seconds.\n");
} else {
    
    if (FD_ISSET(sockfd, &readfds)) {
		read(sockfd,buf1,sizeof (buf1));
		printf("buf1 %x \n",buf1);
	    printf("buf1 %s \n",buf1);
		//break;
    }
	sleep(1);

    if (FD_ISSET(sockfd1, &readfds)) {
	   read(sockfd1,buf2,sizeof(buf2));
		printf("buf2 %x \n",buf2);
		printf("buf2 %s \n",buf2);
		//break;
    }
} 
}
    close(sockfd);
	return 0;
}

