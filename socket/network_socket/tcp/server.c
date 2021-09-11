#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
   int sockfd, new_sockfd; //new_sockfd: accept() would return a new socket fd
   struct sockaddr_storage clientInfo; //for client
   int addrlen = sizeof(clientInfo);
   char msg[256] = { "Hello this is server..................\n" };
   char resp[100] = { "0" };
   int resp_len = 0;
   int status;
   char *port;

   // New methiod to init socket info+++
   struct addrinfo hints, *res, *ptr;
   memset(&hints, 0, sizeof(hints));
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_PASSIVE; //自動填ip

   if (argc < 2) {
	 fprintf(stderr, "Wrong args. Please specify bind port in arg\n");
	 exit(1);
   }

   port = argv[1];

   printf("Specify listening port: %s\n", port);
   if (getaddrinfo(NULL, port, &hints, &res) != 0) { //Use getaddrinfo to init
	   fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
	   exit(1);
   }

   char ipstr[INET6_ADDRSTRLEN];
   struct sockaddr_in *ipv4;
   struct sockaddr_in6 *ipv6;

   for (ptr = res; ptr != NULL; ptr = ptr->ai_next) {
       if (ptr->ai_family == AF_INET) { //IPv4
           ipv4 = (struct sockaddr_in *) ptr->ai_addr;
       } else { //IPv6
           ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
       }

       inet_ntop(ptr->ai_family, &ipv4->sin_addr, ipstr, sizeof(ipstr));
       printf("ipstr=%s\n", ipstr);
   }

   sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
   // New method ---
  
   //bind socket to the port we specify in getaddrinfo()
   bind(sockfd, res->ai_addr, res->ai_addrlen);

   listen(sockfd, 5);

   printf("waiting for accept..\n");
   new_sockfd = accept(sockfd, (struct sockaddr*) &clientInfo, &addrlen);
   
   if (new_sockfd) {
       printf("A client connects!\n");
	   while(1) {
	       resp_len = recv(new_sockfd, resp, sizeof(resp), 0);
	       if (resp_len != 0) {
                   printf("Receive message (len=%d) from client: %s\n", resp_len, resp);
	       } else {
                   printf("No message from client.\n");
		   close(new_sockfd);
		   break;
	       }
	   }
   }
   printf("Server Done\n");
   return 0;
}
