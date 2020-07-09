#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main() {
   int sockfd, new_sockfd;
   struct sockaddr_storage clientInfo; //for client
   int addrlen = sizeof(clientInfo);
   char msg[256] = {"Hello this is server..................\n"};
   char rcvmsg[100] = {"0"};
   int rcv_msglen = 0;

   // New methiod to init socket info+++
   struct addrinfo hints, *res;
   memset(&hints, 0, sizeof(hints));
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_PASSIVE;
   getaddrinfo(NULL, "5566", &hints, &res); //Use getaddrinfo to init
   sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
   // New method ---
   
   //bind socket to the port we specify in getaddrinfo()
   bind(sockfd, res->ai_addr, res->ai_addrlen);

   listen(sockfd, 5);

   new_sockfd = accept(sockfd, (struct sockaddr*) &clientInfo, &addrlen);
   
   if (new_sockfd) {
       printf("Coonect success!\n");
	   while(1) {
	       rcv_msglen = recv(new_sockfd, rcvmsg, sizeof(rcvmsg), 0);
	       printf("rcvmsg=%s\n", rcvmsg);
	   }
   }
   return 0;
}
