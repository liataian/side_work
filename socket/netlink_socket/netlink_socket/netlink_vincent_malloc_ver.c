#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/module.h>
#include <linux/rtnetlink.h>
#include <linux/netlink.h>

#define NETLINK_VINCENT 31

char *cli_msg = "Hello! This is VINCENT from user space";

int main() {
    //Create socket using self define protocol (Need make sure kernel module was inserted first.)
    int sk = socket(AF_NETLINK, SOCK_RAW, NETLINK_VINCENT); //Self define 
    if (sk < 0) { //Coressponding kernel module was not inserted
	    printf("Failed to create socket\n");
	    return -1;
    }

    //Step 1: Init client(Me) structure
    struct sockaddr_nl cli_addr; //a.k.a my name
    memset(&cli_addr, 0, sizeof(cli_addr));
    cli_addr.nl_family = AF_NETLINK;
    cli_addr.nl_pid = getpid(); //We could define any number but default consider pid as our source port number
    cli_addr.nl_groups = 0;

    //Step 2: Bind this socket
    if (bind(sk, (struct sockaddr*)&cli_addr, sizeof(cli_addr)) < 0) {
        printf("Failed to bind netlink socket: %s\n", (char*)strerror(errno));
        close(sk);
        return 1;
    }

    //Step 3: Init Server(Kernel) structure
    struct sockaddr_nl server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.nl_family = AF_NETLINK;
    server_addr.nl_pid = 0; //Zero means kernel
    server_addr.nl_groups = 0; //Zero means unicast

    //Step 4: Init message buffer
    char buf[1024];
    struct iovec iov;
    iov.iov_base = buf;
    iov.iov_len = sizeof(buf);

    //Step 5: Init netlink message header
    struct nlmsghdr *nlh = (struct nlmsghdr*)buf; //nlh always points to buf
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_len = sizeof(buf);
    nlh->nlmsg_type = 5566;

    //Step 6: Copy our message string into payload part
    strcpy(NLMSG_DATA(nlh), cli_msg);

    printf("[Before 1] Message to kernel is '%s' , message type is %d, length is %d\n", (char*)NLMSG_DATA(nlh), nlh->nlmsg_type, nlh->nlmsg_len);
    //Step 7: Init protocol header (to destination)
    struct msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_name = &server_addr;
    msg.msg_namelen = sizeof(server_addr);
    msg.msg_iov = &iov; //io vector (put our header & payload into it)
    msg.msg_iovlen = 1; //io size

    //Step 8: Send message to kernel
    printf("Send message '%s' \n", (char*)NLMSG_DATA(nlh));
    ssize_t snd_status = sendmsg(sk, &msg, 0);
    if (snd_status < 0) {
        printf("Failed to send message to kernel. Error is %s", strerror(errno));
	close(sk);
    }
    printf("[Before 2] Message to kernel is '%s' \n", (char*)NLMSG_DATA(nlh));

    //Step 9: Waiting for reponse messages from kernel    
    //memset(nlh, 0, sizeof(nlh));
    ssize_t rcv_status = recvmsg(sk, &msg, 0);
    printf("[Before 3] Message to kernel is '%s' \n", (char*)NLMSG_DATA(nlh));

#if 1
    //Try to extrace buff manually using another nlh
    struct nlmsghdr *res_msg = (struct nlmsghdr*)buf;
    printf("[After] Message from kernel is %s, type is %d\n", (char*)NLMSG_DATA(res_msg), res_msg->nlmsg_type); //type is not 24 as we define. 24 is protocol
#endif

    if (rcv_status < 0) {
        printf("Failed to receive message from kernel. Error is %s", strerror(errno));
	close(sk);
	return -1;
    }

    //Step 10: Check message name length
    if (msg.msg_namelen != sizeof(cli_addr)) {
        printf("Invalid length of the sender address struct\n");
        close(sk);
	return -1;
    }

    printf("Received message '%s' . Pid of kernel is %d, type is %d\n", (char*)NLMSG_DATA(nlh), nlh->nlmsg_pid, nlh->nlmsg_type);
    close(sk);
    return 0;
}
