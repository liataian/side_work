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
	int bytesent;
	int status;
	pid_t mypid = getpid();
	char msg[64];
	int msglen = snprintf(msg, sizeof(msg), "%s %d\n", "Hello this is msg from client process: ", mypid);

	memset(&client_info, 0, sizeof(client_info));
	client_info.ai_family = AF_INET;
	client_info.ai_socktype = SOCK_STREAM;
	//int errno = getaddrinfo("www.google.com", "5566", &client_info, &remote_info); //This will send DNS query
	if (getaddrinfo("localhost", "5566", &client_info, &remote_info) != 0) {
	   fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
	   return -1;
	}
	
	//要建立socket前可以都先用getaddrinfo來填充remote端資訊
	sockfd = socket(remote_info->ai_family, remote_info->ai_socktype, remote_info->ai_protocol);

	int retry = 3;
	int err = 0;
	while (retry) {
		err = connect(sockfd, remote_info->ai_addr, remote_info->ai_addrlen);
		if (err != 0) {
		    retry--;
                    printf("Failed to connect to server, retry remain %d time\n", retry);
		    usleep(1000000);
		} else {
		    break;
		}
	}

	int count = 10;

	if (!err) {
            while(count > 0) {
                bytesent = send(sockfd, msg, msglen, 0);
		if (bytesent != msglen) {
                    printf("Send msg incompletely, only sent %d bytes.\n", bytesent);
		}
		printf("Send msg (len=%d)done...\n", msglen);
		    count--;
		    usleep(1000000);
	    }
	} else {
	    return -1;
    }
    close(sockfd);
    freeaddrinfo(remote_info);
    return 0;
}
