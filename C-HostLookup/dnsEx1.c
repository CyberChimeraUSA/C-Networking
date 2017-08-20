#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>

int main(int argc, char *argv[]) {

struct hostent *host_info;
struct in_addr *address;

//struct addrinfo *address_info, hints, *p, i;

char input[20];

//not secure but just for demo, use strncpy 
strcpy(input,argv[1]);
printf("Will do a DNS query on : %s \n",input);

host_info = gethostbyname(input);
address = (struct in_addr *) (host_info->h_addr);
printf("%s has address %s \n", input, inet_ntoa(*address));

}





