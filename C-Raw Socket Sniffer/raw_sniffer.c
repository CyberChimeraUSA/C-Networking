#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


void dump(const unsigned char *data_buffer, const unsigned int length);

int main (void)
{

    int i, recv_length, fd;
  u_char buffer[10000];
  if((fd = socket(AF_INET,SOCK_RAW,IPPROTO_TCP))==-1)
     perror("-1 Return \n");
  for(i=0;i<3;i++){
    recv_length = recv(fd,buffer,10000,0);
    printf("Got a %d byte packet\n", recv_length);
    dump(buffer, recv_length);  
    
}
}

void dump(const unsigned char *data_buffer, const unsigned int length) {
   unsigned char byte;
   unsigned int i, j;
   for(i=0; i < length; i++) {
     byte = data_buffer[i];
     printf("%02x ", data_buffer[i]); 
     if(((i%16)==15) || (i==length-1)) {
       for(j=0; j < 15-(i%16); j++)
       printf(" ");
       printf("| ");
        for(j=(i-(i%16)); j <= i; j++) { 
         byte = data_buffer[j];
         if((byte > 31) && (byte < 127)) 
         printf("%c", byte);
     else
      printf(".");
     }
     printf("\n"); 
    } 
   } 
}
