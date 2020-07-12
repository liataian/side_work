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
static struct proc_dir_entry *port_file;
static uint32_t count = 0;
static uint32_t *port;

//For /proc/drop_udp/drop_count +++
static int drop_count_show(struct seq_file *sf, void *data) {
     seq_printf(sf, "%d\n", count);
     return 0;
}

#if 0
//user會用ubuf告知想讀多少data
//kernel要負責把user要讀的data大小塞回給user，並回傳user說我塞了多少給你
static ssize_t drop_count_read(struct file *file, char __user *ubuf, size_t ubuf_size, loff_t *pos) {
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

static int drop_count_show(struct seq_file *sf, void *data) {
     seq_printf(sf, "%d\n", count);
     return 0;
}

static int drop_count_open(struct inode *inode, struct file *file) {
    return single_open(file, drop_count_show, NULL);
}

static const struct file_operations drop_count_fops =
{
    .owner = THIS_MODULE,
    .open = drop_count_open,
    .read = seq_read,
};
#endif

//如果對於drop_count每次都只想讀，除了drop_count_show仍然要自己實現之外，其餘全部操作可用DEFINE_SHOW_ATTRIBUTE(drop_count)取代
DEFINE_SHOW_ATTRIBUTE(drop_count);

//For /proc/drop_udp/drop_count ---




//For /proc/drop_udp/port +++
static int port_show(struct seq_file *sf, void *data) {
     seq_printf(sf, "%u\n", *port);
     return 0;
}

static int port_open(struct inode *inode, struct file *file) {
    return single_open(file, port_show, NULL);
}

static ssize_t port_write(struct file *file, const char __user *ubuf, size_t ubuf_size, loff_t *pos) {
    char buf[128];
    int num, len;
    port = kmalloc(sizeof(uint32_t), GFP_KERNEL); //dynamic allocate
    printk("Enter port_write\n");
    //確認是否是第一次call write(pos=0), 並且確認是否user提供的buffer size是否小於我們要回傳的buffer size
    if (*pos > 0 || ubuf_size > 1024)
        return -EFAULT; //end of file

    if (copy_from_user(buf, ubuf, ubuf_size)) {
        return -EFAULT;
    }

    num = sscanf(buf, "%u", port);
    //printk("num=%d\n", num);

    if (num != 1) //只允許一個值
        return -EFAULT;

    len = strlen(buf);
    *pos = len; //紀錄已經write過

    return len;
}

static const struct file_operations port_fops =
{
    .owner = THIS_MODULE,
    .open  = port_open,
    .read = seq_read,
    .write = port_write, //自己定義write
};
//For /proc/drop_udp/port ---

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

    //Step1: 註冊netfilter hook function
    err = nf_register_net_hook(&init_net, &vincent_drop_udp_ops);

    if (err < 0) {
        printk(KERN_ERR "%s: can't register hooks.(%d)\n", __func__, err);
        return err;
    }

    //Step2: 建立 "/proc/drop_udp/" 目錄
    drop_udp_dir = proc_mkdir("drop_udp", NULL);

    if (!drop_udp_dir) {
        printk(KERN_ERR "%s Fail to create drop udp dir\n", __func__);
        return 1;
    }

    //Step3: 建立 "/proc/drop_udp/drop_count" 檔，用於讀取現在drop多少data (也可以用proc_create_data，多一個參數)
    //第三個參數代表parent dir，也就是要在哪個資料夾之下建立新資料夾，如果是在/proc目錄下建立資料夾，parent為NULL(直接用proc_mkdir)
    drop_count_file = proc_create("drop_count", 0644, drop_udp_dir, &drop_count_fops);

    if (!drop_count_file) {
        printk(KERN_ERR "%s Fail to create file: drop_file\n", __func__);
        return 1;
    }

    //Step4: 建立 "/proc/drop_udp/port" 檔，用於讓user指定想要drop的port
    port_file = proc_create("drop_port", 0666, drop_udp_dir, &port_fops);

    if (!port_file) {
        printk(KERN_ERR "%s Fail to create file: port_file\n", __func__);
        return 1;
    }

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
