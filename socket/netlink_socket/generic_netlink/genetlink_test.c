#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netlink/netlink.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <linux/nl80211.h>
#include <net/if.h>

#if 0
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
#endif

static int finish_handler(struct nl_msg *msg, void *arg) {
        int *ret = arg;
        *ret = 0;
        return NL_SKIP;
}

static int getWifiIface(struct nl_msg *msg, void *arg) {
    printf("Callback getWifiInterface\n");
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), 0);
	if(tb[NL80211_ATTR_IFNAME]) {
		printf("Wifi Iface name=%s\n", nla_get_string(tb[NL80211_ATTR_IFNAME]));
	} else printf("Nothing parse\n");

    if(tb[NL80211_ATTR_SCAN_FREQUENCIES]) {
	    printf("Scan frequency=%s\n", nla_get_string(tb[NL80211_ATTR_SCAN_FREQUENCIES]));
	} else printf("Nothing parse...\n");

    int *ret = arg;
    *ret = 0;
    return NL_SKIP;
}

int main() {
  struct nl_sock *nl_sk;
  struct nl_msg *nl_msg;
  struct nl_msg *nl_msg2;
  struct nl_cb *cb = NULL;
  cb = nl_cb_alloc(NL_CB_CUSTOM);

  //Allocate space for netlink socket
  nl_sk = nl_socket_alloc();
  if (!nl_sk)
		  goto clean;

  //Init netlink socket and bind it
  if(nl_connect(nl_sk, NETLINK_GENERIC) != 0)
		  goto clean;

  //Get generic netlink family id (can use "genl-ctrl-list" to check)
  int nl80211_id = genl_ctrl_resolve(nl_sk, "nl80211");
  if (nl80211_id < 0) {
		  goto clean;
  } else {
		  printf("nl80211_id=0x%x\n", nl80211_id);
  }
  int err = 1;
  nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, getWifiIface, &err);
  //nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, finish_handler, &err);
  nl_socket_modify_cb(nl_sk, NL_CB_MSG_OUT, NL_CB_DEBUG, NULL, NULL);


  //================= message 1 ===================
  //Build netlnk message content (message1)
  nl_msg = nlmsg_alloc(); //Allocate space for netlink message
  if (!nl_msg)
		  goto clean;

  if (!genlmsg_put(nl_msg, NL_AUTO_PORT, NL_AUTO_SEQ, nl80211_id, 0, NLM_F_DUMP, NL80211_CMD_GET_INTERFACE, 0)) { //Build nlmsghdr & genlmsghdr
//		|| nla_put_string(nl_msg, CTRL_ATTR_FAMILY_NAME, "nl80211")) { //Add attr & payload
		  printf("Something wrong ...\n");
		  nlmsg_free(nl_msg);
		  return -1;
  } else {
		  printf("Add NL80211_CMD_GET_INTERFACE OK\n");
  }
  //================= message 1 ==================

  //================= message 2 ==================
  //Get Tx/Rx rate (message2)
  nl_msg2 = nlmsg_alloc();
  if (!nl_msg2)
		  goto clean;
  if (!genlmsg_put(nl_msg2, NL_AUTO_PORT, NL_AUTO_SEQ, nl80211_id, 0, NLM_F_DUMP, NL80211_CMD_GET_STATION, 0)) {
		  printf("Something wrong ...\n");
		  nlmsg_free(nl_msg2);
		  return -1;
  } else {
		  printf("Add NL80211_CMD_GET_STATION OK\n");
  }


  //================= message 2 ==================
  //
  //int mcid = nl_get_multicast_id(nl_sk, "nl80211", "scan");
  //printf("multicast id of nl80211 scan is %d\n", mcid);

  struct nlmsghdr *nl_msghdr = nlmsg_hdr(nl_msg);
  struct genlmsghdr *genl_msghdr = nlmsg_data(nl_msghdr);

  /*+++These two are the same*/
  //struct nlattr *nl_attr = genlmsg_data(genl_msghdr);
  struct nlattr *nl_attr = nlmsg_attrdata(nl_msghdr, NLA_HDRLEN);
  //---These two are the same
  
  char *str = nla_data(nl_attr);
  
  printf("cmd=%d, nla_type=%d, payload=%s\n",
			genl_msghdr->cmd, nl_attr->nla_type, str);

  if (nl_send_auto(nl_sk, nl_msg) < 0) {
     printf("Send wrong ...\n");
	 nlmsg_free(nl_msg);
	 return -1;
  } else {
       printf("Send OK\n");
  }

  while (err > 0) {
   printf("Begin...\n");
   int res = nl_recvmsgs(nl_sk, cb);
   if (res < 0) {
       printf("recv wrong...\n");
   }
  }

  printf("err=%d\n", err);
  if(nl_sk) {
   nl_close(nl_sk);
   nl_socket_free(nl_sk);
  }
  return 0;

clean:
  if (nl_sk) {
	  nl_close(nl_sk);
      nl_socket_free(nl_sk);
  }
  return -1;
}
