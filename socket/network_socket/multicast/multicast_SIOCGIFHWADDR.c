#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include "utils.h"

int main(int argc, char *argv[]) {
    int rawSockFd;
    int byteSent;
    int status;
    unsigned char msg[128];
    int ret;

    memset(msg, 0, sizeof(msg));

    printf("Note: need as root to execute\n");
    if (argc != 4) {
        fprintf(stderr, "Usage: \n sudo ./multicast_client <interface> <dest_mac> <ether_type>\n");
	exit(1);
    }

    struct eth_frame *eth_pkt = (struct eth_frame *) malloc(sizeof(struct eth_frame)); //給我一個struct eth_frame的空間

    memset(eth_pkt, 0, sizeof(*eth_pkt));

    //TODO: 為何先初始化h_source再初始化h_dest，則h_source前三個byte會變為0????
    //1 0 5e 7f ff fa 0 0 0 30 25 1a ...
    //先初始化h_dest則h_source才正常?

    //Step 1. init destination mac (multicast mac)
    char *dest_mac = argv[2];
    set_dest_mac(dest_mac, eth_pkt->h_dest);

    //Step 2. init source mac (my mac on interface)
    char *if_name = argv[1];
    ret = set_source_mac(if_name, eth_pkt->h_source);
    if (ret < 0) {
        fprintf(stderr, "Interface %s is not up. \n", if_name);
	exit(1);
    }

#if 0
    for (int i=0; i<sizeof(eth_pkt->h_source); i++) {
	    printf("eth_pkt->h_source[%d]= %02x\n", i, eth_pkt->h_source[i]);
    }
#endif

    //Step 3. init ethernet type we want to use (ex: 0x9000 is ETH_P_LOOPBACK)
    unsigned int ether_type;
    sscanf(argv[3], "0x%x", &ether_type);
    printf("ethernet type is 0x%x\n", ether_type);
    eth_pkt->h_proto = htons(ether_type); //we need to convert it to network byte order.

    //Step 4. init payload we want to send
    char payload[20] = { "This is Vincent\n" } ;
    memcpy(eth_pkt->payload, payload, sizeof(payload));

    //Step 5. init RAW socket
    if ((rawSockFd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) { //refer include/linux/if_ether.h
        fprintf(stderr, "Failed to create socket\n");
	exit(1);
    }

    //Step 6. Bind address
    /*
      struct sockaddr_ll {
        unsigned short  sll_family;
        __be16          sll_protocol;
        int             sll_ifindex;
        unsigned short  sll_hatype;
        unsigned char   sll_pkttype;
        unsigned char   sll_halen;
        unsigned char   sll_addr[8];
      };
    */
    struct sockaddr_ll addr;
    memset(&addr, 0x0, sizeof(addr));
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htons(ETH_P_ALL);
    addr.sll_ifindex = if_nametoindex(if_name);
    if (bind(rawSockFd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        fprintf(stderr, "Failed to bind socket\n");
	exit(1);
    }

    //Step 7. Copy our packet to buffer and send it
    printf("sizeof(eth_pkt)=%ld, sizeof(*eth_pkt)=%ld, sizeof(struct eth_frame)=%ld, sizeof(eth_pkt->h_dest)=%ld\n", 
		    sizeof(eth_pkt), sizeof(*eth_pkt), sizeof(struct eth_frame), sizeof(eth_pkt->h_dest));
    memcpy(msg, eth_pkt, sizeof(*eth_pkt));

    //Dump packet contect to check
    dump_pkt_content(msg, sizeof(msg));

    while(1) {
	byteSent = send(rawSockFd, &msg, sizeof(msg), 0);
	printf("Sent %d bytes. \n", byteSent);
	usleep(3000000);
    }
    free(eth_pkt);
    close(rawSockFd);
    return 0;
}
