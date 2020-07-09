#include <errno.h>
#include <stdio.h>
#include <memory.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/rtnetlink.h>
#include <sys/types.h>
#include <unistd.h>

struct nl_req { //A request always contains...
  struct nlmsghdr hdr; //A header
  struct rtmsg rtm; //A message, RTM_*ROUTE系列使用
};

void parse_route_attributes(struct rtattr *tb[], int max, struct rtattr *rta, int len) {
    while(RTA_OK(rta, len)) {
        if (rta->rta_type <= max) {
            tb[rta->rta_type] = rta;
        }
        rta = RTA_NEXT(rta, len);
    }
}

void parse_route_entry(struct nlmsghdr *nlh) {
    struct rtmsg *rtm;
    rtm = (struct rtmsg *)NLMSG_DATA(nlh); //Get a route entry
    int len = nlh->nlmsg_len;
    struct rtattr *tb[RTA_MAX+1];
    memset(tb, 0, sizeof(struct rtattr *)*(RTA_MAX+1));

    int table=0, oif=0;
    char dst[40], gw[40], prefsrc[40];
    memset(dst, 0, sizeof(dst));
    memset(gw, 0, sizeof(gw));
    memset(prefsrc, 0, sizeof(prefsrc));

    if (rtm->rtm_table == RT_TABLE_MAIN) { //Only care entries in main table
        parse_route_attributes(tb, RTA_MAX, RTM_RTA(rtm), len); //Parse all attributes of this entry

        if (tb[RTA_TABLE]) { //table name
            table = *(int *)RTA_DATA(tb[RTA_TABLE]);
        }

        if (tb[RTA_GATEWAY]) { //gateway
            inet_ntop(AF_INET, RTA_DATA(tb[RTA_GATEWAY]), gw, sizeof(gw));
        }

        if (tb[RTA_DST]) { //destination address
            inet_ntop(AF_INET, RTA_DATA(tb[RTA_DST]), dst, sizeof(dst));
        }
        if (tb[RTA_OIF]) { //output interface index
            oif = *(int *)RTA_DATA(tb[RTA_OIF]);
        }
        if (tb[RTA_PREFSRC]) { //prefer source address
            inet_ntop(AF_INET, RTA_DATA(tb[RTA_PREFSRC]), prefsrc, sizeof(prefsrc));
        }
        printf("table=%-5d, dst=%-15s, gw=%-15s, oif=%-5d, prefsrc=%-10s\n", table, dst, gw, oif, prefsrc); //加"-"可以靠左對齊
    }
}

int main() {

    //Step 1: Create netlink socket
    int nl_sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (nl_sock < 0) {
        printf("Failed to create netlink socket: %s\n", (char*)strerror(errno));
        return -1;
    }

    //Step 2: Init local
    struct sockaddr_nl local; {
        memset(&local, 0, sizeof(local));
        local.nl_family = AF_NETLINK;
        local.nl_pid = getpid();
        local.nl_groups = 0; //unicast
    }

    //Step 3: bind socket
    if (bind(nl_sock, (struct sockaddr *)&local, sizeof(local)) < 0) {
        printf("Failed to bind netlink socket: %s\n", (char*)strerror(errno));
        close(nl_sock);
        return 1;
    }

    //Step 4: Init remote(kernel)
    struct sockaddr_nl remote; {
        memset(&remote, 0, sizeof(remote));
        remote.nl_family = AF_NETLINK;
        remote.nl_pid = 0; //kernel
        remote.nl_groups = 0;
    }

    //Step 5: Init request
    struct nl_req req;
    memset(&req, 0, sizeof(req));
    //rt netlink header
    req.hdr.nlmsg_type = RTM_GETROUTE;
    req.hdr.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)); 
    req.hdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_DUMP;
    req.hdr.nlmsg_seq = 1;
    req.hdr.nlmsg_pid = getpid();
    //rt netlink message
    req.rtm.rtm_family = AF_INET; //interested in IP-related interface
    req.rtm.rtm_table = RT_TABLE_MAIN;

    //Step 6: Store request in a iov buffer
    struct iovec iov;
    memset(&iov, 0, sizeof(iov));
    iov.iov_base = &req.hdr;
    iov.iov_len = req.hdr.nlmsg_len;

    //Step 7: Construct final message
    struct msghdr msg; {
        memset(&msg, 0, sizeof(msg));
        msg.msg_name = &remote;
        msg.msg_namelen = sizeof(remote);
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1; //only 1 iov
    }

    //Step 8: send message to kernel
    sendmsg(nl_sock, &msg, 0);
    printf("Send RTM_GETROUTE msg done\n");
   
#if 1
    //Step 9: Init message buffer to receive response    
    char reply[8192];
    struct iovec iov_reply;
    memset(&iov_reply, 0, sizeof(iov_reply));
    iov_reply.iov_base = &reply;
    iov_reply.iov_len = sizeof(reply);

    struct msghdr msg_reply; {
        memset(&msg_reply, 0, sizeof(msg_reply));
        msg_reply.msg_name = &remote;
        msg_reply.msg_namelen = sizeof(remote);
        msg_reply.msg_iov = &iov_reply;
        msg_reply.msg_iovlen = 1; //only 1 iov
    }
#endif
    //Step 8: receive response from kernel
    while(1) {
        struct nlmsghdr *nlh = NULL;
        int len = recvmsg(nl_sock, &msg_reply, 0); //blocking... until receive response
        printf("Get response, len=%d\n", len);

        //check status
        if (len < 0) {
            if (errno == EINTR || errno == EAGAIN) {
                printf("sleep 250ms...\n");
                usleep(250000);
                continue;
            }
            printf("Failed to read netlink: %s\n", (char*)strerror(errno));
            continue;
        }

        if (msg.msg_namelen != sizeof(local)) { //check message length, just in case
            printf("Invalid length of the sender address struct\n");
            continue;
        }

        //First read all netlink message header
        for (nlh = (struct nlmsghdr*)reply; NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len)) {
            int nlh_len = nlh->nlmsg_len;
            switch(nlh->nlmsg_type) {
                case NLMSG_DONE:
                    printf("NLMSG_DONE\n");
                    break;
                case RTM_NEWROUTE:
                    parse_route_entry(nlh);
                    break;
                default: 
                    printf("Should not enter here by default\n");
                    break;
            }
        }
    }
    close(nl_sock);
    return 0;
}
