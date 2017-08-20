#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<sys/socket.h>
#include<errno.h> 
#include<netdb.h> 
#include<arpa/inet.h>

 int main()
 {
	int sockfd;  
    struct addrinfo hints, *results, *p;
    struct sockaddr_in *ip_access;
    int rv;
	char *hostname;
    char ip[100], inputVal[100];
	 
	printf("Enter a Domain Name: \n");
	scanf("%s",inputVal);
	//not good practice
	strcpy(hostname,inputVal);
	 
	//zero out structure
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; 
    hints.ai_socktype = SOCK_STREAM;
 
    if ( (rv = getaddrinfo( hostname , "domain" , &hints , &results)) != 0) 
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
	
 //loops through structure using ai_next
    for(p = results; p != NULL; p = p->ai_next) 
    {
		//pass data into sockaddr_in struct
        ip_access = (struct sockaddr_in *) p->ai_addr;

		printf("IP address is %s: \n",inet_ntoa( ip_access->sin_addr ) );	
		
    }

    freeaddrinfo(results); 
   
    printf("\n");
     	
	
 
	 
	 
	 
 }