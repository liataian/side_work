#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

int main() {
	struct addrinfo client_info, *remote_info;
	int sockfd;
    char *msg = "Hello this is client.......YAYAYA";
	int msglen = strlen(msg);
	int bytesent;

	memset(&client_info, 0, sizeof(client_info));
	client_info.ai_family = AF_INET;
	client_info.ai_socktype = SOCK_STREAM;
	//int errno = getaddrinfo("www.google.com", "5566", &client_info, &remote_info); //This will send DNS query
	int errno = getaddrinfo("10.96.144.9", "5566", &client_info, &remote_info);
	if (errno) printf("err=%s\n", gai_strerror(errno));
	
	//要建立socket前可以都先用getaddrinfo來填充remote端資訊
	sockfd = socket(remote_info->ai_family, remote_info->ai_socktype, remote_info->ai_protocol);
	int err = connect(sockfd, remote_info->ai_addr, remote_info->ai_addrlen);

	int count = 10;
	if (!err) {
		while(count>0) {
		    bytesent = send(sockfd, msg, msglen, 0);
		    if (bytesent != msglen) {
		        printf("Send msg incompletely, only sent %d bytes.\n", bytesent);
		    }
		    printf("Send msg (len=%d)done...\n", msglen);
			count--;
			usleep(3000000);
		}
	} else {
	    return -1;
    }
    close(sockfd);
    freeaddrinfo(remote_info);
	return 0;
}

#if 0
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

   if (argc < 4) {
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
#endif
