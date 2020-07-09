#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv4/ip_tables.h>

/*
 * 這個module會用來讀取UDP，並且將總共讀了多少data寫到/proc/drop_udp/drop_count
 */


static struct proc_dir_entry *drop_udp_dir;
static struct proc_dir_entry *drop_count_file;
static uint32_t count = 0;

static int drop_file_show(struct seq_file *sf, void *data) {
     seq_printf(sf, "%d\n", count);
     return 0;
}

static int drop_file_open(struct inode *inode, struct file *file) {
    return single_open(file, drop_file_show, NULL);
}

//user會用ubuf告知想讀多少data
//kernel要負責把user要讀的data大小塞回給user，並回傳user說我塞了多少給你
static ssize_t drop_file_read(struct file *file, char __user *ubuf, size_t ubuf_size, loff_t *pos) {
    char buf[256]; //kernel的buffer
    int len = 0;
    
    printk("This is drop_file_read, ubuf_size=%ld\n", ubuf_size);

    if (*pos > 0 || ubuf_size < 1024) //確認是否是第一次call read(pos=0), 並且確認是否user提供的buffer size是否小於我們要回傳的buffer size
        return 0; //end of file

    len += sprintf(buf, "%d\n", count); //回傳目前讀到多少data

    if (copy_to_user(ubuf, buf, len)) { //從kernel buffer塞回user buffer
        return -EFAULT;   
    }
    *pos = len;
    return len;
}

#if 1
static const struct file_operations drop_file_ops =
{
    .owner = THIS_MODULE,
    .open = drop_file_open,
    .read = seq_read,
    //.write = drop_file_write, //不提供write
};
#endif

static unsigned int vincent_drop_udp_output(void *priv, struct sk_buff *skb, const struct nf_hook_state *state) {
    struct iphdr *iph = ip_hdr(skb);
    struct tcphdr *tcph = tcp_hdr(skb);
    __be32 src_addr = iph->saddr;
    __be32 dst_addr = iph->daddr;
    __be16 src_port = ntohs(tcph->source);
    __be16 dst_port = ntohs(tcph->dest);

    if (skb!=NULL && iph!=NULL && iph->protocol==IPPROTO_UDP) { //only UDP
        // UDP header size = 8 bytes，IP header size = 20 bytes
        // 所以tcpdump抓到的packet size = (skb->len)-8-20
	printk("%s %pI4:%u->%pI4:%u, skb->len=%d, skb->mac_len=%d\n",
		__func__, &src_addr, src_port, &dst_addr, dst_port, skb->len, skb->mac_len); //UDP header size = 8 bytes
        count += skb->len; //存data size
	//return NF_DROP; //Drop it!!!
    }

    return NF_ACCEPT;
}

//定義要在哪個chain上做事
static const struct nf_hook_ops vincent_drop_udp_ops = {
	.hook      = vincent_drop_udp_output, //hook function
	.pf        = NFPROTO_IPV4,
	.hooknum   = NF_INET_POST_ROUTING, //at POSTROUTING chain
	.priority  = NF_IP_PRI_LAST,
};


static int __init vincent_drop_udp_init(void)
{
    int err = 0;
    printk("Enter vincent_drop_UDP_init");

    //註冊netfilter hook function
    err = nf_register_net_hook(&init_net, &vincent_drop_udp_ops);

    if (err < 0) {
        printk(KERN_ERR "%s: can't register hooks.(%d)\n", __func__, err);
        return err;
    }

    //建立目錄
    drop_udp_dir = proc_mkdir("drop_udp", NULL);
    if (!drop_udp_dir) {
        printk(KERN_ERR "%s Fail to create drop udp dir\n", __func__);
        return 1;
    }
#if 1
    //建立檔案
    drop_count_file = proc_create("drop_count", 0644, drop_udp_dir, &drop_file_ops); //也可以用proc_create_data，多一個參數

    if (!drop_count_file) {
        printk(KERN_ERR "%s Fail to create file: drop_file\n", __func__);
        return 1;
    }
#endif
    return 0;
}

static void __exit vincent_drop_udp_deinit(void)
{
    printk("Enter vincent_drop_UDP_deinit");
    nf_unregister_net_hook(&init_net, &vincent_drop_udp_ops);
    proc_remove(drop_udp_dir);       
}

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Vincent Chu");
module_init(vincent_drop_udp_init);
module_exit(vincent_drop_udp_deinit);
