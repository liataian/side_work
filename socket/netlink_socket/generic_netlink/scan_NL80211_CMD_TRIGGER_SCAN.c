#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netlink/netlink.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <linux/nl80211.h>
#include <net/if.h>

#ifndef MAC2STR
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#endif

struct scan_state {
    int done;
    int abort;
};

struct handler_args {  // For family_handler() and nl_get_multicast_id().
    const char *group;
    int id;
};

//此處的nl_msg參數就是kernel回傳的netlink message，我們要把它parse出來
int callback_of_scan(struct nl_msg *msg, void *arg) {
	struct nlmsghdr *msg_hdr = nlmsg_hdr(msg); //用netlink message取得netlink message header位置 
	struct genlmsghdr *genlmsg_hdr = nlmsg_data(msg_hdr); //用netlink message header位置取得generic netlink message header位置
	struct scan_state *state = arg;

        printf("Enter %s\n", __func__);

        //TODO
        //我應該要建立兩個socket，一個用來聽發送的netlink msg，一個用來聽接收的netlink msg
        //現在我只用一個socket負責聽發送跟接收的，所以發送跟接收都會聽到，不好區分
	if (genlmsg_hdr->cmd == NL80211_CMD_SCAN_ABORTED) {
		printf("We got NL80211_CMD_SCAN_ABORTED... (%d)\n", genlmsg_hdr->cmd);
		state->done = 1;
		state->abort = 1;
	} else if(genlmsg_hdr->cmd == NL80211_CMD_NEW_SCAN_RESULTS) {
		printf("We got NL80211_CMD_NEW_SCAN_RESULTS! (%d)\n", genlmsg_hdr->cmd);
		state->abort = 0;
		state->done = 1;
	} else {
		printf("We got something we are not interested. (%d)\n", genlmsg_hdr->cmd);
	}
	return NL_SKIP; 
}

int callback_of_get_scan_result(struct nl_msg *msg, void *arg) {
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    const uint8_t *ie = NULL; //IE就是probe response中Tagged parameter中的欄位，byte為單位
    size_t ie_len = 0;
    uint32_t status = 0;
    uint32_t assoc_freq = 0;
    uint32_t assoc_channel = 0;
    uint8_t mac[6];
    int signal_level = 0;
    uint32_t last_seen = 0; //ms
    uint32_t static scan_result_count = 1;
    char ssid[32]; //for parsing ssid in IE
    struct nlattr *tb[NL80211_ATTR_MAX + 1]; //用來存第一層nla
    struct nlattr *bss[NL80211_BSS_MAX + 1]; //用來存第二層nla (因為bss屬於nested nla)

    //BSS一定要檢查type
    static struct nla_policy bss_policy[NL80211_BSS_MAX + 1] = {
        [NL80211_BSS_BSSID] = { .type = NLA_UNSPEC },
        [NL80211_BSS_FREQUENCY] = { .type = NLA_U32 },
        [NL80211_BSS_INFORMATION_ELEMENTS] = { .type = NLA_UNSPEC },
        [NL80211_BSS_STATUS] = { .type = NLA_U32 },
    };

    //先不給定nla_policy
    nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);

    if (tb[NL80211_ATTR_IFNAME]) {
        printf("IFACE=%s, ", nla_get_string(tb[NL80211_ATTR_IFNAME]));
    }

    //確定一下nla中有沒有包含bss資訊+++
    if(!tb[NL80211_ATTR_BSS]) {
        printf("BSS info is missing.\n");
	goto FAIL;
    }

    //有的話就parse bss資訊
    if (nla_parse_nested(bss, NL80211_BSS_MAX, tb[NL80211_ATTR_BSS], bss_policy)) {
        printf("Failed to parse BSS nested nla.\n");
	goto FAIL;
    }

    //每call一次callback代表上報一個scan result;
    printf("%u. ", scan_result_count++);

    //取得SSID, 只能從IE中取得, IE是probe response或beacon中的資訊(Tagged parameter)
    //非常多資訊, 基本上就是直接parse packet中的欄位, 可挑自己想抓的
    if (bss[NL80211_BSS_INFORMATION_ELEMENTS]) {
        ie = nla_data(bss[NL80211_BSS_INFORMATION_ELEMENTS]);
	ie_len = nla_len(bss[NL80211_BSS_INFORMATION_ELEMENTS]);
	//printf("IE[0]=%u, IE[1]=%u, ie_len=%lu ", ie[0], ie[1], ie_len);
        //SSID名稱是從第3個byte開始，長度為是第2個byte紀錄(也就是ie[1])，需要額外包含"\0"
	snprintf(ssid, ie[1]+1, "%s", ie+2);
	printf("SSID=%s, ", ssid);
    }

    //取得BSSID
    if (bss[NL80211_BSS_BSSID]) {
        memcpy(mac, nla_data(bss[NL80211_BSS_BSSID]), 6);
	printf("BSSID=%02x:%02x:%02x:%02x:%02x:%02x, ", MAC2STR(mac));
    }

    //取得frequency
    if (bss[NL80211_BSS_FREQUENCY]) {
        assoc_freq = nla_get_u32(bss[NL80211_BSS_FREQUENCY]);
        printf("%uMHz, ", assoc_freq);
	if (assoc_freq >= 2412 && assoc_freq <= 2472) { //2.4G (channel 1~11)
            assoc_channel = (assoc_freq - 2407)/5;
	    printf("ch%u, ", assoc_channel);
	} else if (assoc_freq >= 5180 && assoc_freq <= 5240) { //5G band 1 (channel 36~48)
	    assoc_channel = (assoc_freq - 5000)/5;
	    printf("ch%u, ", assoc_channel);
	} else if (assoc_freq >= 5745 && assoc_freq <= 5845) { //5G band 4 (channel 149~169)
	    assoc_channel = (assoc_freq - 5000)/5;
	    printf("ch%u, ", assoc_channel);
	} else {
            //..
	}
    }

    //取得singal
    if (bss[NL80211_BSS_SIGNAL_MBM]) {
        signal_level = nla_get_u32(bss[NL80211_BSS_SIGNAL_MBM]);
        printf("%ddbm, ", signal_level/100); //mBm to dBm
    }

#if 0
    //取得beacon interval
    if (nla_get_u16(bss[NL80211_BSS_BEACON_INTERVAL])) {
	printf("BI=%ums, ", nla_get_u16(bss[NL80211_BSS_BEACON_INTERVAL]));
    }
#endif

    //距離上一次掃到此SSID過了多少ms
    if (nla_get_u32(bss[NL80211_BSS_SEEN_MS_AGO])) {
        last_seen = nla_get_u32(bss[NL80211_BSS_SEEN_MS_AGO]);
	printf("last_seen=%ums ", last_seen);
    }

    //parse BSS info---
    if (bss[NL80211_BSS_STATUS]) {
        status = nla_get_u32(bss[NL80211_BSS_STATUS]);
	//找出我們目前連上的AP
	if (status == NL80211_BSS_STATUS_ASSOCIATED) {
	    printf("(connected)");
	}
    }
    printf("\n");

//以下這些不是從scan result這個netlink packet可以獲得的資訊
#if 0
    if (tb[NL80211_ATTR_IFNAME]) {
        printf("IFACE=%s, ", nla_get_string(tb[NL80211_ATTR_IFNAME]));
    }

    if (tb[NL80211_ATTR_WIPHY_CHANNEL_TYPE]) {
	printf("CH_TYPE=%u, ", nla_get_u32(tb[NL80211_ATTR_WIPHY_CHANNEL_TYPE])); 
    }

    if (tb[NL80211_ATTR_CHANNEL_WIDTH]) {
	printf("WIDTH=%d, ", nla_get_u32(tb[NL80211_ATTR_CHANNEL_WIDTH]));
    }
    if (tb[NL80211_ATTR_CENTER_FREQ1]) {
	printf("CENTER_FREQ1=%uMHz, ", nla_get_u32(tb[NL80211_ATTR_CENTER_FREQ1]));
    }

    if (tb[NL80211_ATTR_CENTER_FREQ2]) {
	printf("CENTER_FREQ2=%uMHz, ", nla_get_u32(tb[NL80211_ATTR_CENTER_FREQ2]));
    }
    if (tb[NL80211_ATTR_VHT_CAPABILITY]) {
        printf("VHT cap, ");
    }
#endif

    return 0;

FAIL:
    return NL_SKIP;
}

//同wpa_supplicant+++ (參考src/drivers/driver_nl80211.c)
static int finish_handler(struct nl_msg *msg, void *arg) {
    // Callback for NL_CB_FINISH.
    int *ret = arg;
    *ret = 0;
    return NL_SKIP;
}

static int ack_handler(struct nl_msg *msg, void *arg) {
    // Callback for NL_CB_ACK.
    int *ret = arg;
    printf("Enter %s\n", __func__);
    *ret = 0;
    return NL_STOP;
}

static int no_seq_check(struct nl_msg *msg, void *arg) {
    // Callback for NL_CB_SEQ_CHECK.
    return NL_OK;
}

//wpa_supplicant簡化版本
static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err, void *arg) {
    // Callback for errors.
    printf("error_handler() called.\n");
    int *ret = arg;
    *ret = err->error;
    return NL_STOP;
}
//同wpa_supplicant---

//參考iw+++
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
    struct handler_args grp = {
	    .group = group,
	    .id = -ENOENT,
    };

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

    ret = nl_send_auto(sock, msg);
    if (ret < 0) goto out;

    ret = 1;

    nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &ret);
    nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &ret);
    nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, family_handler, &grp);

    while (ret > 0)
	    nl_recvmsgs(sock, cb);

    if (ret == 0) 
	    ret = grp.id;

    nla_put_failure:
        out:
            nl_cb_put(cb);
        out_fail_cb:
            nlmsg_free(msg);
            return ret;
}
//參考iw---

int trigger_scan(struct nl_sock *socket, int drv_id, unsigned int iface_idx) {
    //建立用+++
    struct nl_msg *msg;
    struct nlattr *ssids;
    //建立用---

    //確認用+++
    struct nlmsghdr *msg_hdr;
    struct genlmsghdr *genlmsg_hdr;
    struct nlattr *idx;
    unsigned int *payload;
    //確認用---
    int ret = -1;

    printf("Enter %s\n", __func__);
    msg = nlmsg_alloc();
    if (!msg) {
        printf("Failed to allocate netlink message.\n");
        return -ENOMEM;
    }

    /*
     *  給定command，同時填充nlmsghdr與genlmsghdr
     */
    //Issue command NL80211_CMD_TRIGGER_SCAN to trigger scan
    if(!genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, drv_id, 0, 0, NL80211_CMD_TRIGGER_SCAN, 0)) {
        printf("Failed to create netlink message header.\n");
	goto fail;
    }


    /*
     *  給定scan所需的參數
     */
    //Add attribute type NL80211_ATTR_IFINDEX to select which iface to use
    if(nla_put_u32(msg, NL80211_ATTR_IFINDEX, iface_idx))
	    goto fail;
    //nl_msg_dump(msg, stdout); //快速dump出整個netlink message格式


    /*
     *  給定scan所需的nested參數
     */
    //TODO 終於搞懂, 給定NL80211_ATTR_SCAN_SSIDS只是告訴driver要額外針對這些SSID發送單獨的probe request，但還是會伴隨broadcast的probe request，所以還是會報全部的scan result
    if (!(ssids = nla_nest_start(msg, NL80211_ATTR_SCAN_SSIDS)) || //Add nested attribute type "NL80211_ATTR_SCAN_SSIDS" to specify SSIDs we want to scan
		    nla_put(msg, 1, 0, "")) {
#if 0
		    nla_put(msg, 1, 8, "AX86U_2G") || //Add specific SSID as attribute value
		    nla_put(msg, 2, 8, "AX86U_5G")) {
#endif
		goto fail;
    }
    nla_put(msg, 1, 0, ""); //scan all SSIDs
    nla_nest_end(msg, ssids);
    //nl_msg_dump(msg, stdout); //快速dump出整個netlink message格式

    /*
     *  發送前確認一下給定指令跟參數是否正確
     */
    msg_hdr = nlmsg_hdr(msg); //用netlink message取得netlink message header位置
    genlmsg_hdr = nlmsg_data(msg_hdr); //用netlink message header位置取得generic netlink message header位置
    idx = genlmsg_data(genlmsg_hdr); //用generic netlink message header位置取得netlink attribute位置
    payload = nla_data(idx); //netlink attribute位置取得netlink attribute payload位置

    printf("Sending: cmd=%d, nla_len=%d, nla_type=%d, iface_idx=%u, payload=%u\n", genlmsg_hdr->cmd, idx->nla_len, idx->nla_type, iface_idx, *payload);

    //Send message
    ret = nl_send_auto(socket, msg);
    if(ret < 0) {
        printf("Failed to send netlink message.\n");
	goto fail;
    } else {
        printf("[NL80211_CMD_TRIGGER_SCAN] Sent %d bytes to the kernel sucessfully.\n", ret);
    }

    nlmsg_free(msg);
    return ret;

fail:
    nlmsg_free(msg);
    return ret;
}

int get_scan_result(struct nl_sock *socket, int drv_id, int iface_idx) {
    struct nl_msg *msg;
    int ret = -1;

    msg = nlmsg_alloc();
    if (!msg) {
        printf("Failed to allocate netlink message.\n");
        return -ENOMEM;
    }

    //Issue command "NL80211_CMD_GET_SCAN" to get scan result
    if(!genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, drv_id, 0, NLM_F_DUMP, NL80211_CMD_GET_SCAN, 0)) {
        printf("Failed to create netlink message header.\n");
	goto fail;
    }

    //Add attribute "NL80211_ATTR_IFINDEX" to select which iface to use
    if(nla_put_u32(msg, NL80211_ATTR_IFINDEX, iface_idx))
	    goto fail;

    //Send message
    ret = nl_send_auto(socket, msg);
    if(ret < 0) {
        printf("Failed to send netlink message.\n");
	goto fail;
    } else {
        printf("[NL80211_CMD_GET_SCAN] Sent %d bytes to the kernel sucessfully.\n", ret);
    }

    nlmsg_free(msg);
    return ret;
fail:
        nlmsg_free(msg);
        return ret;
}

int main() {
    struct nl_sock *socket = nl_socket_alloc();
    int drv_id;
    unsigned int iface_idx = if_nametoindex("wlan0"); //get iface id (net/if.h)
    int ret;
    int err = -ENOMEM;
    struct nl_cb *cb = NULL;
    struct scan_state state = { //用來紀錄trigger scan後kernel callback的結果
	    .done = 0,
	    .abort = 0
    };

    //Init and bind general netlink socket
    genl_connect(socket);

    //Get nl80211 family id
    drv_id = genl_ctrl_resolve(socket, "nl80211");

    //Init netlink callback
    cb = nl_cb_alloc(NL_CB_DEFAULT);
    if (!cb) {
        printf("Failed to allocate netlink callbacks.\n");
        return -ENOMEM;
    }

    //先設定用於處理kernel的回應的callback, 第五個參數可傳任意變數進去+++ (可設定想要收到哪些，這邊基本上都收)
    //TODO
    //我應該要建立兩個socket，一個用來聽發送的netlink msg，一個用來聽接收的netlink msg
    //現在我只用一個socket負責聽發送跟接收的，所以發送跟接收都會聽到 (同wpa_supplicant???)
    err = 1; //後續這些callback被呼叫時會修改這個err
    nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, callback_of_scan, &state); //自訂的callback handler可用NL_CB_VALID指定
    nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
    nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err); //用於確定command是否下成功 
    nl_cb_set(cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, no_seq_check, NULL); //No sequence checking for multicast messages.
    nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &err); //用於確定錯誤狀況
    //先設定用於處理kernel的回應的callback---


    //Join multicast group "scan" to listen scan result
    int mcid = nl_get_multicast_id(socket, "nl80211", "scan");
    if (mcid >= 0) {
	    //"callback_of_scan" won't be called without nl_socket_add_membership
	    mcid = nl_socket_add_membership(socket, mcid);
    }

    if (mcid < 0) {
	    printf("Failed to add multicast membership for scan events (%d)", mcid);
    }

    //Issue NL80211_CMD_TRIGGER_SCAN command to kernel
    ret = trigger_scan(socket, drv_id, iface_idx);
    if (ret < 0) {
        printf("Failed to trigger scan. (%d)\n", ret);
        return ret;
    }

    while (err > 0) { //一旦哪個callback被trigger, err就會被修改
	    int res = nl_recvmsgs(socket, cb); //第一個trigger的callback應該是ack_handler. 正常發送後kernel先回ack，err就會設0 
	    if (res < 0) {
		    printf("nl_recvmsgs() returned %d (%s).\n", res, nl_geterror(-res));
	    }
    }

    //等待收到kernel的NL80211_CMD_NEW_SCAN_RESULTS或是NL80211_CMD_SCAN_ABORTED(可能另一個process也在trigger scan)
    while (!state.done) { //callback_of_scan會負責決定
	    nl_recvmsgs(socket, cb);
    }

    if (state.abort) {
	    printf("Failed to trigger scan due to abort.\n");
	    return -1;
    }

    printf("Scan done.\n");

    //TODO 這裡可以想想怎樣動態給予自訂callback(參考supplicant的send_and_recv_msgs)，而不是每做一個動作就modify callback一次
    nl_socket_modify_cb(socket, NL_CB_VALID, NL_CB_CUSTOM, callback_of_get_scan_result, NULL);

    //Issue NL80211_CMD_GET_SCAN command to kernel
    ret = get_scan_result(socket, drv_id, iface_idx);
    if (ret < 0) {
        printf("Failed to get scan result. (%d)\n", ret);
        return ret;
    }
    printf("Get scan result done.\n");

    //nl_recvmsgs_default使用的是socket的default cb! 是從nl_socket_modify_cb設定的
    ret = nl_recvmsgs_default(socket);

    if (ret < 0) {
        printf("ERROR: nl_recvmsgs_default() returned %d (%s).\n", ret, nl_geterror(-ret));
        return ret;
    }
    return 0;
}
