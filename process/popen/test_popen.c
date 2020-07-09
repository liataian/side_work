#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
        FILE *fp = popen("ip route show table main", "r");
        char buff[128];
        char tmp[100][20];
        //char *tencent_ip[100];
        int count = 0;
        if (fp == NULL) {
                printf("[IP_RULE_TUN0] popen() error!\n");
                return 1;
        } else {
                //output like this: "121.51.37.101 dev tun0 proto static scope link"
                while(fgets(buff, sizeof(buff), fp) != NULL) {
                        printf("buff=%s", buff);
                        char *start = buff;
                        char *end = strchr(buff, ' ');
                        int offset = end-start;
                        memset(tmp[count], '\0', sizeof(tmp[count]));
                        strncpy(tmp[count], start, offset);
                        printf("tmp[%d]=%s\n\n", count, tmp[count]);
                //tencent_ip[count] = tmp;
                //ALOGE("[IP_RULE_TUN0] tencent_ip[%d]=%s", count, tencent_ip[count]);
                        count++;
                }
        }
        pclose(fp);
        return 0;
}
