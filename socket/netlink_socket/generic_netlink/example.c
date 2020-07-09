#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netlink/netlink.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <linux/nl80211.h>
#include <net/if.h>

int callback_round = 0;

struct trigger_results {
    int done;
    int aborted;
};


struct handler_args {  // For family_handler() and nl_get_multicast_id().
    const char *group;
    int id;
};


static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err, void *arg) {
    // Callback for errors.
    printf("error_handler() called.\n");
    int *ret = arg;
    *ret = err->error;
    return NL_STOP;
}


static int finish_handler(struct nl_msg *msg, void *arg) {
    // Callback for NL_CB_FINISH.
    int *ret = arg;
    *ret = 0;
    return NL_SKIP;
}


static int ack_handler(struct nl_msg *msg, void *arg) {
    // Callback for NL_CB_ACK.
    int *ret = arg;
    *ret = 0;
    return NL_STOP;
}


static int no_seq_check(struct nl_msg *msg, void *arg) {
    // Callback for NL_CB_SEQ_CHECK.
    return NL_OK;
}


static int family_handler(struct nl_msg *msg, void *arg) {
    // Callback for NL_CB_VALID within nl_get_multicast_id(). From http://sourcecodebrowser.com/iw/0.9.14/genl_8c.html.
    struct handler_args *grp = arg;
    struct nlattr *tb[CTRL_ATTR_MAX + 1];
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct nlattr *mcgrp;
    int rem_mcgrp;

    nla_parse(tb, CTRL_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);

    if (!tb[CTRL_ATTR_MCAST_GROUPS]) return NL_SKIP;

    nla_for_each_nested(mcgrp, tb[CTRL_ATTR_MCAST_GROUPS], rem_mcgrp) {  // This is a loop.
        struct nlattr *tb_mcgrp[CTRL_ATTR_MCAST_GRP_MAX + 1];

        nla_parse(tb_mcgrp, CTRL_ATTR_MCAST_GRP_MAX, nla_data(mcgrp), nla_len(mcgrp), NULL);

        if (!tb_mcgrp[CTRL_ATTR_MCAST_GRP_NAME] || !tb_mcgrp[CTRL_ATTR_MCAST_GRP_ID]) continue;
        if (strncmp(nla_data(tb_mcgrp[CTRL_ATTR_MCAST_GRP_NAME]), grp->group,
                nla_len(tb_mcgrp[CTRL_ATTR_MCAST_GRP_NAME]))) {
            continue;
                }

        grp->id = nla_get_u32(tb_mcgrp[CTRL_ATTR_MCAST_GRP_ID]);
        break;
    }

    return NL_SKIP;
}


int nl_get_multicast_id(struct nl_sock *sock, const char *family, const char *group) {
    // From http://sourcecodebrowser.com/iw/0.9.14/genl_8c.html.
    struct nl_msg *msg;
    struct nl_cb *cb;
    int ret, ctrlid;
    struct handler_args grp = { .group = group, .id = -ENOENT, };

    msg = nlmsg_alloc();
    if (!msg) return -ENOMEM;

    cb = nl_cb_alloc(NL_CB_DEFAULT);
    if (!cb) {
        ret = -ENOMEM;
        goto out_fail_cb;
    }

    ctrlid = genl_ctrl_resolve(sock, "nlctrl");

    genlmsg_put(msg, 0, 0, ctrlid, 0, 0, CTRL_CMD_GETFAMILY, 0);

    ret = -ENOBUFS;
    NLA_PUT_STRING(msg, CTRL_ATTR_FAMILY_NAME, family);

    ret = nl_send_auto_complete(sock, msg);
    if (ret < 0) goto out;

    ret = 1;

    nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &ret);
    nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &ret);
    nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, family_handler, &grp);

    while (ret > 0) nl_recvmsgs(sock, cb);

    if (ret == 0) ret = grp.id;

    nla_put_failure:
        out:
            nl_cb_put(cb);
        out_fail_cb:
            nlmsg_free(msg);
            return ret;
}


void mac_addr_n2a(char *mac_addr, unsigned char *arg) {
    int i, l;

    l = 0;
    for (i = 0; i < 6; i++) {
	//	printf("arg[%d]=%x\n", i, arg[i]);
        if (i == 0) {
            sprintf(mac_addr+l, "%02x", arg[i]);
            l += 2;
        } else {
            sprintf(mac_addr+l, ":%02x", arg[i]);
            l += 3;
        }
    }
}


void print_ssid(unsigned char *ie, int ielen) {
    uint8_t len;
    uint8_t *data;
    int i;
    while (ielen >= 2 && ielen >= ie[1]) {
        if (ie[0] == 0 && ie[1] >= 0 && ie[1] <= 32) {
            len = ie[1];
            data = ie + 2;
            for (i = 0; i < len; i++) {
                if (isprint(data[i]) && data[i] != ' ' && data[i] != '\\') {
					printf("%c", data[i]);
				} else if (data[i] == ' ' && (i != 0 && i != len -1)) {
					printf(" ");
				} else {
					printf("\\x%.2x", data[i]);
				}
            }
            break;
        }
        ielen -= ie[1] + 2;
        ie += ie[1] + 2;
    }
}


static int callback_trigger(struct nl_msg *msg, void *arg) {
    // Called by the kernel when the scan is done or has been aborted.
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct trigger_results *results = arg;

    printf("Got something.\n");
    //printf("%d\n", arg);
    //nl_msg_dump(msg, stdout);

    if (gnlh->cmd == NL80211_CMD_SCAN_ABORTED) {
        printf("Got NL80211_CMD_SCAN_ABORTED.\n");
        results->done = 1;
        results->aborted = 1;
    } else if (gnlh->cmd == NL80211_CMD_NEW_SCAN_RESULTS) {
        printf("Got NL80211_CMD_NEW_SCAN_RESULTS.\n");
        results->done = 1;
        results->aborted = 0;
    }  // else probably an uninteresting multicast message.

    return NL_SKIP;
}


static int callback_dump(struct nl_msg *msg, void *arg) {
    // Called by the kernel with a dump of the successful scan's data. Called for each SSID.
	callback_round++;
    //nl_msg_dump(msg, stdout);
	printf("\ncallback_round %d\n", callback_round);
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    char mac_addr[20];
    struct nlattr *tb[NL80211_ATTR_MAX + 1];
    struct nlattr *bss[NL80211_BSS_MAX + 1];

    static struct nla_policy bss_policy[NL80211_BSS_MAX + 1] = {
        [NL80211_BSS_TSF] = { .type = NLA_U64 },
        [NL80211_BSS_FREQUENCY] = { .type = NLA_U32 },
        [NL80211_BSS_BSSID] = { },
        [NL80211_BSS_BEACON_INTERVAL] = { .type = NLA_U16 },
        [NL80211_BSS_CAPABILITY] = { .type = NLA_U16 },
        [NL80211_BSS_INFORMATION_ELEMENTS] = { },
        [NL80211_BSS_SIGNAL_MBM] = { .type = NLA_U32 },
        [NL80211_BSS_SIGNAL_UNSPEC] = { .type = NLA_U8 },
        [NL80211_BSS_STATUS] = { .type = NLA_U32 },
        [NL80211_BSS_SEEN_MS_AGO] = { .type = NLA_U32 },
        [NL80211_BSS_BEACON_IES] = { },
    };

    //Parse all attributes in each message into tb[]
    nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);
    if (!tb[NL80211_ATTR_BSS]) {  //NL80211_ATTR_BSS == 47
        printf("bss info missing!\n");
        return NL_SKIP;
	}

    if (tb[NL80211_ATTR_IFINDEX]) {
	    printf("NL80211_ATTR_IFINDEX: %u (len=%u bytes), len=%u, type=%u\n",
						nla_get_u32(tb[NL80211_ATTR_IFINDEX]),
						nla_len(tb[NL80211_ATTR_IFINDEX]),
						tb[NL80211_ATTR_IFINDEX]->nla_len,
						tb[NL80211_ATTR_IFINDEX]->nla_type);
	}

    if (tb[NL80211_ATTR_SCAN_FREQUENCIES]) {
	    printf("NL80211_ATTR_SCAN_FREQUENCIES: %u, len=%u, type=%u\n",
						nla_get_u32(tb[NL80211_ATTR_SCAN_FREQUENCIES]),
						tb[NL80211_ATTR_SCAN_FREQUENCIES]->nla_len,
						tb[NL80211_ATTR_SCAN_FREQUENCIES]->nla_type);
	}
    
#if 0
    for (int i=0; i<NL80211_ATTR_MAX; i++) {
	   if (tb[i]) {
			  printf("tb[%d] has value\n",i);
	   }
	}
#endif
	//Parse all payloads in each attribute into bss[]
    if (nla_parse_nested(bss, NL80211_BSS_MAX, tb[NL80211_ATTR_BSS], bss_policy)) {
        printf("failed to parse nested attributes!\n");
        return NL_SKIP;
    }
    if (!bss[NL80211_BSS_BSSID])
		return NL_SKIP;

	//Check current BSS status
	if (bss[NL80211_BSS_STATUS]) {
	    int status = nla_get_u32(bss[NL80211_BSS_STATUS]);
        if (status == NL80211_BSS_STATUS_ASSOCIATED && bss[NL80211_BSS_FREQUENCY]) {
                uint32_t assoc_freq = nla_get_u32(bss[NL80211_BSS_FREQUENCY]);
                printf("nl80211: Associated on this channel: %u MHz\n", assoc_freq);
        }
	}
#if 0
    for (int i=0; i<NL80211_BSS_MAX; i++) {
	   if (bss[i]) {
		printf("bss[%d] has value\n", i);
	   }
	}
#endif
	uint8_t *ie, *beacon_ie;
    size_t ie_len, beacon_ie_len;
    if (bss[NL80211_BSS_INFORMATION_ELEMENTS]) {
	    ie = nla_data(bss[NL80211_BSS_INFORMATION_ELEMENTS]);
		ie_len = nla_len(bss[NL80211_BSS_INFORMATION_ELEMENTS]);
	} else {
		return NL_SKIP;
	}

    // Start printing.
    mac_addr_n2a(mac_addr, nla_data(bss[NL80211_BSS_BSSID])); //BSSID is MAC
    printf("%s, ", mac_addr);
    printf("%d MHz, ", nla_get_u32(bss[NL80211_BSS_FREQUENCY]));
//ASUS_BSP+++
    int signal_level = nla_get_u32(bss[NL80211_BSS_SIGNAL_MBM]);
	signal_level /= 100; /* mBm to dBm */
    printf("%d dbm, ", signal_level);
	int beacon_interval = nla_get_u16(bss[NL80211_BSS_BEACON_INTERVAL]);
	printf("%d ms, ", beacon_interval);
//ASUS_BSP---
    print_ssid(nla_data(bss[NL80211_BSS_INFORMATION_ELEMENTS]), nla_len(bss[NL80211_BSS_INFORMATION_ELEMENTS]));
    printf("\n");

    return NL_SKIP;
}


int do_scan_trigger(struct nl_sock *socket, int if_index, int driver_id) {
    // Starts the scan and waits for it to finish. Does not return until the scan is done or has been aborted.
    struct trigger_results results = {
			.done = 0,
			.aborted = 0
	};
    struct nl_msg *msg;
    struct nl_cb *cb;
    struct nl_msg *ssids_to_scan;
    int err;
	int error;
    int ret;
    int mcid = nl_get_multicast_id(socket, "nl80211", "scan");
    nl_socket_add_membership(socket, mcid);  // Without this, callback_trigger() won't be called.

    // Allocate the messages and callback handler.
    msg = nlmsg_alloc();
    if (!msg) {
        printf("ERROR: Failed to allocate netlink message for msg.\n");
        return -ENOMEM;
    }
    ssids_to_scan = nlmsg_alloc();
    if (!ssids_to_scan) {
        printf("ERROR: Failed to allocate netlink message for ssids_to_scan.\n");
        nlmsg_free(msg);
        return -ENOMEM;
    }
    cb = nl_cb_alloc(NL_CB_DEFAULT);
    if (!cb) {
        printf("ERROR: Failed to allocate netlink callbacks.\n");
        nlmsg_free(msg);
        nlmsg_free(ssids_to_scan);
        return -ENOMEM;
    }

    // Setup the messages and callback handler.
	//Add command to trigger scan
    genlmsg_put(msg, 0, 0, driver_id, 0, 0, NL80211_CMD_TRIGGER_SCAN, 0);

    //ASUS_BSP+++
    nla_put_u32(msg, NL80211_ATTR_IFINDEX, if_index);
	struct nlattr *ssids;
	ssids = nla_nest_start(msg, NL80211_ATTR_SCAN_SSIDS);
    nla_put(msg, 1, 12, "BSP_WIFI_2G");
    nla_put(msg, 2, 12, "BSP_WIFI_5G");
    nla_nest_end(msg, ssids);
    //ASUS_BSP---


    //nla_put_u32(msg, NL80211_ATTR_IFINDEX, if_index);
	//Specify SSID we want to scan
	/*
    nla_put(ssids_to_scan, 1, 0, "");  // Scan all SSIDs.
    nla_put_nested(msg, NL80211_ATTR_SCAN_SSIDS, ssids_to_scan);  // Add message attribute, which SSIDs to scan for.
    nlmsg_free(ssids_to_scan);  // Copied to `msg` above, no longer need this.
    */
	//Add 5 kind of callbacks
    nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, callback_trigger, &results);
    nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &err);
    nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
    nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err);
    nl_cb_set(cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, no_seq_check, NULL); //No sequence checking for multicast messages.

    //Specify frequency we want to scan
	/*
	unsigned int freq[1] = {2412};
    struct nlattr *freqs;
    freqs = nla_nest_start(msg, NL80211_ATTR_SCAN_FREQUENCIES);
	if (freqs == NULL) {
		printf("Something wrong........\n");
		return -1;
	}
    for (int i = 0; i < 1; i++) {
        printf("Add scan frequency %u MHz into attribute payload\n", freq[i]);
        if (nla_put_u32(msg, i+1, freq[i]))
            return -1;
    }
    nla_nest_end(msg, freqs);
	*/

//ASUS_BSP+++ Debug message format
    //nl_msg_dump(msg, stderr);
#if 1
	//if ((error = nl_socket_modify_cb(socket, NL_CB_MSG_IN, NL_CB_DEBUG, NULL, NULL)) < 0)
      //  printf("Could not register debug cb for incoming packets.\n");
    if ((error = nl_socket_modify_cb(socket, NL_CB_MSG_OUT, NL_CB_DEBUG, NULL, NULL)) < 0)
        printf("Could not register debug cb for outgoing packets.\n");
#endif
//ASUS_BSP---
    // Send NL80211_CMD_TRIGGER_SCAN to start the scan. The kernel may reply with NL80211_CMD_NEW_SCAN_RESULTS on
    // success or NL80211_CMD_SCAN_ABORTED if another scan was started by another process.
    err = 1;
    ret = nl_send_auto(socket, msg);  // Send the message.
    printf("NL80211_CMD_TRIGGER_SCAN sent %d bytes to the kernel.\n", ret);
    printf("Waiting for scan to complete...\n");
    while (err > 0) {
		ret = nl_recvmsgs(socket, cb);  // First wait for ack_handler(). This helps with basic errors.
	}
    if (err < 0) {
        printf("WARNING: err has a value of %d.\n", err);
    }
    if (ret < 0) {
        printf("ERROR: nl_recvmsgs() returned %d (%s).\n", ret, nl_geterror(-ret));
        return ret;
    }
    while (!results.done) nl_recvmsgs(socket, cb);  // Now wait until the scan is done or aborted.
    if (results.aborted) {
        printf("ERROR: Kernel aborted scan.\n");
        return 1;
    }
    printf("Scan is done.\n");

    // Cleanup.
    nlmsg_free(msg);
    nl_cb_put(cb);
    nl_socket_drop_membership(socket, mcid);  // No longer need this.
    return 0;
}


int main() {
    struct nl_sock *socket = nl_socket_alloc();
	//Init and bind
    genl_connect(socket);
	int err;
	int ret;
    int driver_id = genl_ctrl_resolve(socket, "nl80211");
    unsigned int if_index = if_nametoindex("wlan0"); //interface
	printf("interface index=%u\n", if_index);
    // Issue NL80211_CMD_TRIGGER_SCAN to the kernel and wait for it to finish.
    err = do_scan_trigger(socket, if_index, driver_id);
    if (err != 0) {
        printf("do_scan_trigger() failed with %d.\n", err);
        return err;
    }
    struct nl_msg *msg = nlmsg_alloc();
    //Add command to get scan result
    genlmsg_put(msg, 0, 0, driver_id, 0, NLM_F_DUMP, NL80211_CMD_GET_SCAN, 0);
    //Add attribute to choose interface
    nla_put_u32(msg, NL80211_ATTR_IFINDEX, if_index);

    //Add callback
    nl_socket_modify_cb(socket, NL_CB_VALID, NL_CB_CUSTOM, callback_dump, NULL);
    //Send message
    ret = nl_send_auto(socket, msg);
    printf("NL80211_CMD_GET_SCAN sent %d bytes to the kernel.\n", ret);

    //Retrieve the kernel's answer. callback_dump() prints SSIDs to stdout.
    ret = nl_recvmsgs_default(socket);
    nlmsg_free(msg);

    if (ret < 0) {
        printf("ERROR: nl_recvmsgs_default() returned %d (%s).\n", ret, nl_geterror(-ret));
        return ret;
    }

    return 0;
}
