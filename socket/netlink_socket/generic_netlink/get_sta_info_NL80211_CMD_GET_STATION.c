#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netlink/netlink.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <linux/nl80211.h>
#include <net/if.h>

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

//同wpa_supplicant+++ (參考src/drivers/driver_nl80211.c)
static int finish_handler(struct nl_msg *msg, void *arg) {
    // Callback for NL_CB_FINISH.
    printf("Enter %s\n", __func__);
    int *ret = arg;
    *ret = 0;
    return NL_SKIP;
}

static int ack_handler(struct nl_msg *msg, void *arg) {
    // Callback for NL_CB_ACK.
    printf("Enter %s\n", __func__);
    int *ret = arg;
    *ret = 0;
    return NL_STOP;
}

//wpa_supplicant簡化版本
static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err, void *arg) {
    // Callback for errors.
    printf("Enter %s\n", __func__);
    int *ret = arg;
    *ret = err->error;
    return NL_STOP;
}
//同wpa_supplicant---


int callback_of_get_interface_index(struct nl_msg *msg, void *arg) {
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct nlattr *tb[NL80211_ATTR_MAX + 1]; //用來存第一層nla
    unsigned int *idx = arg;

    nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);

    if (tb[NL80211_ATTR_IFNAME]) {
        printf("IFACE=%s, ", nla_get_string(tb[NL80211_ATTR_IFNAME]));
    }
    
    if (tb[NL80211_ATTR_IFINDEX]) {
        //printf("INDEX=%u, ", nla_get_u32(tb[NL80211_ATTR_IFINDEX]));
        *idx = nla_get_u32(tb[NL80211_ATTR_IFINDEX]);
    }

    printf("\n");

    return NL_SKIP;
}

int get_interface_index(struct nl_sock *socket, int drv_id) {
    struct nl_msg *msg;
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
    //Issue command NL80211_CMD_GET_INTERFACE to get station info (Must use NLM_F_DUMP!!)
    if(!genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, drv_id, 0, NLM_F_DUMP, NL80211_CMD_GET_INTERFACE, 0)) {
        printf("Failed to create netlink message header.\n");
	goto fail;
    }

    //Send message
    ret = nl_send_auto(socket, msg);

    if(ret < 0) {
        printf("Failed to send netlink message.\n");
	goto fail;
    } else {
        printf("[NL80211_CMD_GET_INTERFACE] Sent %d bytes to the kernel sucessfully.\n", ret);
    }

    nlmsg_free(msg);
    return ret;

fail:
    nlmsg_free(msg);
    return ret;
}

int callback_of_get_sta_info(struct nl_msg *msg, void *arg) {
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct nlattr *tb[NL80211_ATTR_MAX + 1]; //用來存第一層nla, 確認是否有sta info
    struct nlattr *tb_info[NL80211_ATTR_MAX + 1]; //用來存第二層nla, 存sta info資訊
    struct nlattr *tb_tx_rate[NL80211_ATTR_MAX + 1]; //用來存第三層nla, 存tx bitrate資訊
    struct nlattr *tb_rx_rate[NL80211_ATTR_MAX + 1]; //用來存第三層nla, 存rx bitrate資訊
    unsigned int *gotit = arg;

    static struct nla_policy info_policy[NL80211_STA_INFO_MAX + 1] = {
        [NL80211_STA_INFO_TX_BYTES] = { .type = NLA_U32 },
        [NL80211_STA_INFO_RX_BYTES] = { .type = NLA_U32 },
        [NL80211_STA_INFO_TX_PACKETS] = { .type = NLA_U32 },
        [NL80211_STA_INFO_RX_PACKETS] = { .type = NLA_U32 },
        [NL80211_STA_INFO_TX_BITRATE] = { .type = NLA_NESTED },
        [NL80211_STA_INFO_RX_BITRATE] = { .type = NLA_NESTED },
        [NL80211_STA_INFO_SIGNAL] = { .type = NLA_U8 },
    };

    static struct nla_policy bitrate_policy[NL80211_RATE_INFO_MAX + 1] = {
        [NL80211_RATE_INFO_BITRATE] = { .type = NLA_U16 },
        [NL80211_RATE_INFO_MCS] = { .type = NLA_U8 },
        [NL80211_RATE_INFO_VHT_MCS] = { .type = NLA_U8 },
        [NL80211_RATE_INFO_VHT_NSS] = { .type = NLA_U8 },
        [NL80211_RATE_INFO_SHORT_GI] = { .type = NLA_FLAG },
    };

    printf("Enter %s\n", __func__);

    //不給定nla_policy
    nla_parse(tb,
              NL80211_ATTR_MAX,
	      genlmsg_attrdata(gnlh, 0),
	      genlmsg_attrlen(gnlh, 0),
	      NULL);

    /*========================================================*/

    //如果NL80211_ATTR_STA_INFO不存在就直接閃了
    if (!tb[NL80211_ATTR_STA_INFO]) {
        printf("STA info is missing.\n");
	goto FAIL;
    }

    //確定有sta info才設定, 讓main裡面處理完後可以break while
    *gotit = 1;

    //NL80211_ATTR_STA_INFO存在的話就parse，記得要從tb[NL80211_ATTR_STA_INFO]開始
    if (nla_parse_nested(tb_info,
			 NL80211_STA_INFO_MAX,
			 tb[NL80211_ATTR_STA_INFO],
			 info_policy)) {
        printf("Failed to parse STA info nested nla.\n");
	goto FAIL;
    }

    if (tb_info[NL80211_STA_INFO_TX_BYTES]) {
        printf("TX_BYTES=%u\n", nla_get_u32(tb_info[NL80211_STA_INFO_TX_BYTES]));
    }

    if (tb_info[NL80211_STA_INFO_TX_PACKETS]) {
        printf("TX_PACKETS=%u\n", nla_get_u32(tb_info[NL80211_STA_INFO_TX_PACKETS]));
    }

    if (tb_info[NL80211_STA_INFO_RX_BYTES]) {
        printf("TX_BYTES=%u\n", nla_get_u32(tb_info[NL80211_STA_INFO_RX_BYTES]));
    }

    if (tb_info[NL80211_STA_INFO_RX_PACKETS]) {
        printf("RX_PACKETS=%u\n", nla_get_u32(tb_info[NL80211_STA_INFO_TX_PACKETS]));
    }

    if (tb_info[NL80211_STA_INFO_CHAIN_SIGNAL]) {
        printf("SIGNAL=%s\n", nla_get_string(tb_info[NL80211_STA_INFO_CHAIN_SIGNAL]));
    }

    if (tb_info[NL80211_STA_INFO_SIGNAL]) { //RSSI要從unsigned char轉成signed char
        printf("SIGNAL=%d dbm(%u dbm)\n", (int8_t)nla_get_u8(tb_info[NL80211_STA_INFO_SIGNAL]), nla_get_u8(tb_info[NL80211_STA_INFO_SIGNAL]));
    }

    /*========================================================*/

    //確定一下有沒有包含Tx bitrate資訊
    if (!tb_info[NL80211_STA_INFO_TX_BITRATE]) {
        printf("Tx bitrate is missing.\n");
	goto FAIL;
    }

    //有的話就parse Tx bitrate資訊
    if (nla_parse_nested(tb_tx_rate,
                        NL80211_RATE_INFO_MAX,
			tb_info[NL80211_STA_INFO_TX_BITRATE],
			bitrate_policy)) {
        printf("Failed to parse Tx bitrate nested nla.\n");
	goto FAIL;
    }

    printf("Tx information:\n");
    if (tb_tx_rate[NL80211_RATE_INFO_BITRATE]) {
        printf("  BITRATE=%u\n", nla_get_u16(tb_tx_rate[NL80211_RATE_INFO_BITRATE]));
    }

    if (tb_tx_rate[NL80211_RATE_INFO_MCS]) {
        printf("  MCS=%u\n", nla_get_u8(tb_tx_rate[NL80211_RATE_INFO_MCS]));
    }

    if (tb_tx_rate[NL80211_RATE_INFO_VHT_MCS]) {
        printf("  VHT_MCS=%u\n", nla_get_u8(tb_tx_rate[NL80211_RATE_INFO_VHT_MCS]));
    }

    if (tb_tx_rate[NL80211_RATE_INFO_VHT_NSS]) {
        printf("  VHT_NSS=%u\n", nla_get_u8(tb_tx_rate[NL80211_RATE_INFO_VHT_NSS]));
    }

    if(nla_get_flag(tb_tx_rate[NL80211_RATE_INFO_40_MHZ_WIDTH])) {
        printf("  VHT40\n");
    }
    if(nla_get_flag(tb_tx_rate[NL80211_RATE_INFO_80_MHZ_WIDTH])) {
        printf("  VHT80\n");
    }
    /*========================================================*/

    //確定一下有沒有包含Rx bitrate資訊
    if (!tb_info[NL80211_STA_INFO_RX_BITRATE]) {
        printf("Tx bitrate is missing.\n");
	goto FAIL;
    }

    //有的話就parse Rx bitrate資訊
    if (nla_parse_nested(tb_rx_rate,
                        NL80211_RATE_INFO_MAX,
			tb_info[NL80211_STA_INFO_RX_BITRATE],
			bitrate_policy)) {
        printf("Failed to parse Rx bitrate nested nla.\n");
	goto FAIL;
    }

    printf("Rx information:\n");
    if (tb_rx_rate[NL80211_RATE_INFO_BITRATE]) {
        printf("  BITRATE=%u\n", nla_get_u16(tb_rx_rate[NL80211_RATE_INFO_BITRATE]));
    }

    if (tb_rx_rate[NL80211_RATE_INFO_MCS]) {
        printf("  MCS=%u\n", nla_get_u8(tb_rx_rate[NL80211_RATE_INFO_MCS]));
    }

    if (tb_rx_rate[NL80211_RATE_INFO_VHT_MCS]) {
        printf("  VHT_MCS=%u\n", nla_get_u8(tb_rx_rate[NL80211_RATE_INFO_VHT_MCS]));
    }

    if (tb_rx_rate[NL80211_RATE_INFO_VHT_NSS]) {
        printf("  VHT_NSS=%u\n", nla_get_u8(tb_rx_rate[NL80211_RATE_INFO_VHT_NSS]));
    }

    if(nla_get_flag(tb_tx_rate[NL80211_RATE_INFO_40_MHZ_WIDTH])) {
        printf("  VHT40\n");
    }

    if(nla_get_flag(tb_tx_rate[NL80211_RATE_INFO_80_MHZ_WIDTH])) {
        printf("  VHT80\n");
    }

    return NL_SKIP;

FAIL:
    return NL_SKIP;
}

int get_station_info(struct nl_sock *socket, int drv_id, unsigned int iface_idx) {
    struct nl_msg *msg;
    int ret = -1;
    unsigned char mac_addr[ETH_ALEN] = { 0x3c, 0x7c, 0x3f, 0xe1, 0x2e, 0x00 };

    printf("Enter %s\n", __func__);

    msg = nlmsg_alloc();
    if (!msg) {
        printf("Failed to allocate netlink message.\n");
        return -ENOMEM;
    }

    /*
     *  給定command，同時填充nlmsghdr與genlmsghdr
     */
    //Issue command NL80211_CMD_GET_STATION to get station info (Must use NLM_F_DUMP!!)
    if(!genlmsg_put(msg,
                    NL_AUTO_PORT,
		    NL_AUTO_SEQ,
		    drv_id,
		    0,
		    NLM_F_DUMP,
		    NL80211_CMD_GET_STATION,
		    0)) {
        printf("Failed to create netlink message header.\n");
	goto fail;
    }

    //Add attribute type NL80211_ATTR_IFINDEX to select which iface to use
    if(nla_put_u32(msg, NL80211_ATTR_IFINDEX, iface_idx)) {
        goto fail;
    }

    //Add attribute type NL80211_ATTR_MAC to give mac address
    if (nla_put(msg, NL80211_ATTR_MAC, ETH_ALEN, mac_addr)) {
        goto fail;
    }

    //Send message
    ret = nl_send_auto(socket, msg);

    if(ret < 0) {
        printf("Failed to send netlink message.\n");
	goto fail;
    } else {
        printf("[NL80211_CMD_GET_STATION] Sent %d bytes to the kernel sucessfully.\n", ret);
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
#if 0
    //寫死版本
    unsigned int iface_idx = if_nametoindex("wlan0"); //get iface id (net/if.h)
#else
    //使用NL80211_CMD_GET_STATION取得interface版本
    unsigned int iface_idx = 0;
#endif
    int ret;
    int err = -ENOMEM;
    unsigned int got = 0;
    struct nl_cb *cb = NULL;
    struct nl_cb *cb2 = NULL;

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

    cb2 = nl_cb_alloc(NL_CB_DEFAULT);
    if (!cb2) {
        printf("Failed to allocate netlink callbacks.\n");
        return -ENOMEM;
    }

    //Step 1: 先取得interface index
    //先設定用於處理kernel的回應的callback (可設定想要收到哪些，先設定啥都收)
    //TODO 關於nl_cb_set最後一個參數, 可用不同參數控制收到啥cb才要做啥
    err = 1; //後續這些callback被呼叫時會修改這個err
    nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
    nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err); //用於確定command是否下成功 
    nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &err); //用於確定錯誤狀況
    nl_cb_set(cb,
	      NL_CB_VALID,
	      NL_CB_CUSTOM,
	      callback_of_get_interface_index,
	      &iface_idx); //自訂的callback handler可用NL_CB_VALID指定

    ret = get_interface_index(socket, drv_id);
    if (ret < 0) {
        printf("Failed to get station info. (%d)\n", ret);
        return ret;
    }

    while (err > 0) { //Whatever callback is triggered, err would be modified to 0
        int res = nl_recvmsgs(socket, cb); //First triggered callback should be ack_handler, because kernel would replies an ACK once got command from userspace. 
        if (res < 0) {
            printf("nl_recvmsgs() returned %d (%s).\n", res, nl_geterror(-res));
        }
    }

    printf("Send NL80211_CMD_GET_INTERFACE done.\n");

    //Wait until callback_of_get_interface_index is called and then idx would > 0
    while (iface_idx == 0) {
        int res = nl_recvmsgs(socket, cb); 
        if (res < 0) {
            printf("nl_recvmsgs() returned %d (%s).\n", res, nl_geterror(-res));
        }
    }

    //Step 2: 用剛剛獲得的interface index去取得sta資訊

#if 0
    /* 再新增一個callback的版本*/
    err = 1; //reset err
    nl_cb_set(cb2, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
    nl_cb_set(cb2, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err); //用於確定command是否下成功 
    nl_cb_err(cb2, NL_CB_CUSTOM, error_handler, &err); //用於確定錯誤狀況
    nl_cb_set(cb2,
              NL_CB_VALID,
	      NL_CB_CUSTOM,
	      callback_of_get_sta_info,
	      &got);
#else
    /* 直接修改socket的callback的版本:
     * 注意這裡修改的是socket->s_cb, 是一個default callback
     * 後續要搭配nl_recvmsgs_default來使用default callback! 因為我們沒有把自訂的callback設定給socket->s_cb
     * nl_recvmsgs都是用自訂的callback, 所以才需要傳自訂callback進去
     */
    nl_socket_modify_cb(socket,
		        NL_CB_VALID,
			NL_CB_CUSTOM,
			callback_of_get_sta_info,
			&got);
#endif

    ret = get_station_info(socket, drv_id, iface_idx);
    if (ret < 0) {
        printf("Failed to get station info. (%d)\n", ret);
        return ret;
    }

#if 0
    /* 再新增一個callback的版本*/
    while (err > 0) {
        int res = nl_recvmsgs(socket, cb2); 
        if (res < 0) {
            printf("nl_recvmsgs() returned %d (%s).\n", res, nl_geterror(-res));
        }
    }
    printf("Send NL80211_CMD_GET_STATION done.\n");

    while (got == 0) {
        int res = nl_recvmsgs(socket, cb2); 
        if (res < 0) {
            printf("nl_recvmsgs() returned %d (%s).\n", res, nl_geterror(-res));
        }
    }
#else
    /* 直接修改socket的callback的版本: */
    while (got == 0) {
        int res = nl_recvmsgs_default(socket);
        if (res < 0) {
            printf("ERROR: nl_recvmsgs_default() returned %d (%s).\n", ret, nl_geterror(-ret));
            return res;
	}
    }
#endif

    return 0;
}
