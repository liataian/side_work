#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>

int main(){
  int fd;
  struct ifreq ifr;

  fd = socket(AF_INET, SOCK_DGRAM, 0);

  ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
  //origin: 02:42:a9:08:c3:0a
  ifr.ifr_hwaddr.sa_data[0] = 0x02;
  ifr.ifr_hwaddr.sa_data[1] = 0x42;
  ifr.ifr_hwaddr.sa_data[2] = 0xa9;
  ifr.ifr_hwaddr.sa_data[3] = 0x08;
  ifr.ifr_hwaddr.sa_data[4] = 0xc3;
  ifr.ifr_hwaddr.sa_data[5] = 0x0a;
  
  strncpy(ifr.ifr_name, "docker0", IFNAMSIZ - 1);
  if(ioctl(fd, SIOCSIFHWADDR, &ifr) != 0){
    perror("ioctl");
    return 1;
  }
}
