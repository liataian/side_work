#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netlink/netlink.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <linux/nl80211.h>
#include <net/if.h>

int trigger_scan_for_all_SSIDs(struct nl_sock *socket, int drv_id, int iface_idx) {
    struct nl_msg *msg = nlmsg_alloc();
    struct nlattr *ssids;

    if (!msg) {
        printf("Failed to allocate netlink message: msg.\n");
        return -ENOMEM;
    }

}

int trigger_scan_for_specific_SSIDs(struct nl_sock *socket, int drv_id, int iface_idx) {
    struct nl_msg *msg;
    struct nlattr *nla_start;

    msg = nlmsg_alloc();
    if (!msg) {
        printf("Failed to allocate netlink message: msg.\n");
        return -ENOMEM;
    }


    //同時填充nlmsghdr與genlmsghdr
    //Issue command "NL80211_CMD_TRIGGER_SCAN" to trigger scan
    genlmsg_put(msg, 0, 0, drv_id, 0, 0, NL80211_CMD_TRIGGER_SCAN, 0);

    //Add attribute type "NL80211_ATTR_IFINDEX" to select which iface to use
    nla_put_u32(msg, NL80211_ATTR_IFINDEX, iface_idx);

    //Add nested attribute type "NL80211_ATTR_SCAN_SSIDS" to select SSIDs we want to scan
    nla_start = nla_nest_start(msg, NL80211_ATTR_SCAN_SSIDS);
    nla_put(msg, 1, 12, "BSP_WIFI_2G");
    nla_put(msg, 2, 12, "BSP_WIFI_5G");
    nla_nest_end(msg, nla_start);


}

int get_scan_result(struct nl_sock *socket, int drv_id, int iface_idx) {
    struct nl_msg *msg;
    int ret;

    msg = nlmsg_alloc();
    if (!msg) {
        printf("Failed to allocate netlink message: msg.\n");
        return -ENOMEM;
    }

    //Issue command "NL80211_CMD_GET_SCAN" to get scan result
    genlmsg_put(msg, 0, 0, driver_id, 0, NLM_F_DUMP, NL80211_CMD_GET_SCAN, 0);

    //Add attribute "NL80211_ATTR_IFINDEX" to select which iface to use
    nla_put_u32(msg, NL80211_ATTR_IFINDEX, iface_idx);

    //Send message
    ret = nl_send_auto(socket, msg);
    printf("Use NL80211_CMD_GET_SCAN to sent %d bytes to the kernel.\n", ret);

    ret = nl_recvmsgs_default(socket);

    nlmsg_free(msg);

    if (ret < 0) {
        printf("ERROR: nl_recvmsgs_default() returned %d (%s).\n", ret, nl_geterror(-ret));
        return ret;
    }

    return 0;
}

int main() {
    struct nl_sock *socket = nl_socket_alloc();
    int drv_id = genl_ctrl_resolve(socket, "nl80211"); //get nl family id
    unsigned int iface_idx = if_nametoindex("wlan0"); //get iface id (net/if.h)
    int err;

    genl_connect(socket); //init and bind general netlink socket

    //Issue NL80211_CMD_TRIGGER_SCAN command to kernel
    err = trigger_scan(socket, if_index, driver_id);
    if (err != 0) {
        printf("Failed to trigger scan. (%d)\n", err);
        return err;
    }

    //Add callback
    nl_socket_modify_cb(socket, NL_CB_VALID, NL_CB_CUSTOM, callback_dump, NULL);

    return 0;
}
