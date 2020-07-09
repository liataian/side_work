#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>

int main(void) {
        struct ifreq ifr;
        int sockfd;
        char *addr;
        sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
        bzero(&ifr, sizeof(ifr));
        //Check if interface is up
        strcpy(ifr.ifr_name, "wlan0");
        ioctl(sockfd, SIOCGIFFLAGS, &ifr); //ioctl
        if (ifr.ifr_flags & IFF_UP)
		printf("wlan0 is up!\n");
        else
		printf("wlan0 is not up\n");

        //Get IP address
        memset(&ifr.ifr_addr, 0, sizeof(ifr.ifr_addr));
        ifr.ifr_addr.sa_family = AF_INET;
        ioctl(sockfd, SIOCGIFADDR, &ifr); //ioctl
        printf("%s\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
        close(sockfd);
        return 0;
}
