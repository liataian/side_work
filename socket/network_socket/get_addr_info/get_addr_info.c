#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

//int getaddrinfo(const char *node, // 例如： "www.example.com" 或 IP
//                const char *service, // 例如： "http" 或 port number
//                                const struct addrinfo *hints,
//                                                struct addrinfo **res);

int main(int argc, char** argv) {
   int status;
   int count = 0;
   struct addrinfo client_info, *server_info, *current_info;
   void *addr;
   char ipstr[INET6_ADDRSTRLEN];

   memset(&client_info, 0, sizeof(client_info));
   client_info.ai_family = AF_INET;
   client_info.ai_socktype = SOCK_STREAM;
   client_info.ai_flags = AI_PASSIVE;

   if (argc < 2) {
	 fprintf(stderr, "Wrong args. Please specify remote address in arg\n");
	 exit(1);
   }

   if ((status = getaddrinfo(argv[1], NULL, &client_info, &server_info)) != 0) {
	   fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
	   exit(1);
   }
   for (current_info = server_info; current_info != NULL; current_info = current_info->ai_next) {
       count++;
       struct sockaddr_in *ipv4 = (struct sockaddr_in *)server_info->ai_addr;
       addr = &(ipv4->sin_addr);
       inet_ntop(current_info->ai_family, addr, ipstr, sizeof(ipstr));
       printf("Remote address %d is %s\n", count, ipstr);
   }
   freeaddrinfo(server_info);
   return 0;
}
