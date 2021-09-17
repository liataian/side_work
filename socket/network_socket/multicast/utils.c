#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <sys/ioctl.h>
#include "utils.h"

void dump_pkt_content(unsigned char *content, int size) {
    //Dump packet contect to check
    printf("\nPacket content+++\n");
    for (int i=0; i < size; i++) {
        printf("%x ", content[i]);
    }
    printf("\nPacket content---\n\n");
}

int set_source_mac(char *interface, unsigned char *mac) {
        int sockfd;
        char *addr;

        struct ifreq ifr;
        bzero(&ifr, sizeof(ifr));

        sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
        strncpy(ifr.ifr_name, interface, IFNAMSIZ-1);
        ioctl(sockfd, SIOCGIFFLAGS, &ifr); //Check if interface is up first

	if (ifr.ifr_flags & IFF_UP) {
	    printf("%s is up. \n", interface);
	} else {
	    close(sockfd);
	    return -1;
	}

        //memset(&ifr.ifr_addr, 0, sizeof(ifr.ifr_addr));
	ifr.ifr_addr.sa_family = AF_INET;
        ioctl(sockfd, SIOCGIFHWADDR, &ifr); //Get mac address
        close(sockfd);

	for (int i=0; i<6; i++) {
	    /*
	     * struct sockaddr { 
	     *     unsigned short  sa_family;  // 2 bytes address family, AF_xxx 
	     *     char       sa_data[14];   // 14 bytes of protocol address 
	     * };
	     */
	    /* 如果char不先轉unsigned char就用%x印出來(等同轉成int印出來), 則:
	     * f8(11111000)因為最高位bit為1，其餘會擴展全補1，因此會變成fffffff8
	     * f4(11110100)因為最高位bit為1，其餘會擴展全補1，因此會變成fffffff4
	     * e3(11100011)因為最高位bit為1，其餘會擴展全補1，因此會變成ffffffe3
	     * 30(00110000)因為最高位bit為0，其餘會擴展全補0，因此會變成00000030
	     * 25(00100101)因為最高位bit為0，其餘會擴展全補0，因此會變成00000025
	     * 1a(00011010)因為最高位bit為0，其餘會擴展全補0，因此會變成0000001a
	     * 最後印出來mac address前三個byte就會有問題
	     */
            //printf("ifr.ifr_hwaddr.sa_data[%d]=%02x\n", i, ifr.ifr_hwaddr.sa_data[i]);
	}

	printf("\n");
	for (int i=0; i<6; i++) {
	    /* 如果char有先轉unsigned char，則無論最高位為1或0都不會擴展，也就是皆為0，則:
	     * f8(11111000)會變成000000f8
	     * f4(11110100)會變成000000f4
	     * e3(11100011)會變成000000e3
	     * 30(00110000)會變成00000030
	     * 25(00100101)會變成00000025
	     * 1a(00011010)會變成0000001a
	     */
            //printf("ifr.ifr_hwaddr.sa_data[%d]=%02x\n", i, (unsigned char) ifr.ifr_hwaddr.sa_data[i]);
	}

	*(mac+0) = (unsigned char) ifr.ifr_hwaddr.sa_data[0];
	*(mac+1) = (unsigned char) ifr.ifr_hwaddr.sa_data[1];
	*(mac+2) = (unsigned char) ifr.ifr_hwaddr.sa_data[2];
	*(mac+3) = (unsigned char) ifr.ifr_hwaddr.sa_data[3];
	*(mac+4) = (unsigned char) ifr.ifr_hwaddr.sa_data[4];
	*(mac+5) = (unsigned char) ifr.ifr_hwaddr.sa_data[5];

        printf("Mac address on %s is %02x:%02x:%02x:%02x:%02x:%02x\n",
			interface,
			(unsigned char) mac[0],
			(unsigned char) mac[1],
			(unsigned char) mac[2],
			(unsigned char) mac[3],
			(unsigned char) mac[4],
			(unsigned char) mac[5]);
        return 0;
}

void set_dest_mac(char *mac_string, unsigned char *dest_mac) {
    //printf("mac_string=%s\n", mac_string);
    sscanf(mac_string, "%02x:%02x:%02x:%02x:%02x:%02x",
		    (unsigned int *) &dest_mac[0], //轉成unsigned是因為不要因為最高位的bit為1(signed)而全補1
		    (unsigned int *) &dest_mac[1],
		    (unsigned int *) &dest_mac[2],
		    (unsigned int *) &dest_mac[3],
		    (unsigned int *) &dest_mac[4],
		    (unsigned int *) &dest_mac[5]);

    printf("Destination mac is %02x:%02x:%02x:%02x:%02x:%02x\n",
		    dest_mac[0], dest_mac[1], dest_mac[2], dest_mac[3], dest_mac[4], dest_mac[5]);
}
