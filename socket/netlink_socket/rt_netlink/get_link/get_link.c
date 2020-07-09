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
  struct ifinfomsg ifmsg; //RTM_*LINK系列使用
};

/* Step 3 */
void parse_link_attributes(struct rtattr *tb[], int max, struct rtattr *rta, int len) {
    while(RTA_OK(rta, len)) {
        if (rta->rta_type <= max) {
            tb[rta->rta_type] = rta;
        }
        rta = RTA_NEXT(rta, len);
    }
}

/* Step 2 */
void parse_link_entry(struct nlmsghdr *nlh) {
    struct ifinfomsg *ifinfo;
    struct rtattr *tb[IFLA_MAX + 1];
    int len = nlh->nlmsg_len;
    char *ifname=NULL, *state=NULL, *running=NULL;

    ifinfo = (struct ifinfomsg *)NLMSG_DATA(nlh);
    memset(tb, 0, sizeof(struct rtattr *)*(RTA_MAX+1));

    parse_link_attributes(tb, IFLA_MAX, IFLA_RTA(ifinfo), len); //Parse all attributes of this entry

    if (tb[IFLA_IFNAME]) { //table name
            ifname = (char *)RTA_DATA(tb[IFLA_IFNAME]);
    }

    if (ifinfo->ifi_flags & IFF_UP) { //UP flag
        state = (char*)"UP";
    } else {
        state = (char*)"DOWN";
    }

    if (ifinfo->ifi_flags & IFF_RUNNING) { //RUNNING flag
        running = (char*)"RUNNING";
    } else {
        running = (char*)"NOT RUNNING";
    }

    printf("Interface%d=%-10s state=%-5s running=%-10s\n", ifinfo->ifi_index, ifname, state, running);

}

/* Step 1 */
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
    req.hdr.nlmsg_type = RTM_GETLINK; //get LINK
    req.hdr.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg)); 
    req.hdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_DUMP;
    req.hdr.nlmsg_seq = 1;
    req.hdr.nlmsg_pid = getpid();

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
    printf("Send RTM_GETLINK msg done\n");
   
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

    //Step 10: receive response from kernel
    while(1) {
        struct nlmsghdr *nlh = NULL;
        int len = recvmsg(nl_sock, &msg_reply, 0); //blocking... until receive response
        //printf("Get response, len=%d\n", len);

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
                    //printf("NLMSG_DONE\n");
                    break;
                case RTM_NEWLINK:
                    parse_link_entry(nlh);
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
