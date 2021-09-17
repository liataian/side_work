#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>

//參考/usr/include/linux/if_ether.h中的struct ethhdr
//由於我們還需要多一個payload欄位，所以自己建立一個新結構
struct eth_frame {
    unsigned char h_dest[ETH_ALEN]; /* destination eth addr */
    unsigned char h_source[ETH_ALEN]; /* source ether addr    */
    __be16 h_proto; /* packet type ID field */    //為big-endian(所以需要使用ntohs()轉換後再讀取)
    char payload[64];
};

void dump_pkt_content(unsigned char *content, int size);
int set_source_mac(char *interface, unsigned char *mac);
void set_dest_mac(char *mac_string, unsigned char *dest_mac);
