#include <linux/bpf.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/in.h>
#include "./include/bpf_endian.h"
#include "./include/bpf_helpers.h"

typedef struct key {
    unsigned int src;
    unsigned int dst;
}KEY;

typedef struct value {
    long drop_bytes;
    long drop_counts;
}VALUE;

//loader載入這個BPF program時會解析此SEC("maps")並call bpf system call (with BPF_MAP_CREATE)直接建立map
struct bpf_map_def SEC("maps") drop_icmp_map = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(KEY), //src & dst ip address
    .value_size = sizeof(VALUE),
    .max_entries = 256,
};

SEC("DROP_ICMP")
static __always_inline int drop_icmp(struct xdp_md *ctx) {
    void *data_start = (void*)(long)ctx->data;
    void *data_end = (void*)(long)ctx->data_end;
    struct ethhdr *eth = data_start;

    if ((void*)(eth+1) > data_end) //ethernet header must smaller than whole data
	    return XDP_ABORTED;

    if (eth->h_proto == bpf_htons(ETH_P_IP)) { //check if there is IP packet in ethernet frame
        struct iphdr *iph = (struct iphdr*)(eth+1);
	if ((void*)(iph+1) > data_end) //IP header must smaller than rest of data
		return XDP_ABORTED;
        //bpf_printk("Got a packet\n");

	//如果是ICMP packet
	if (iph->protocol == IPPROTO_ICMP) {
	    KEY key;
	    key.src = iph->saddr;
	    key.dst = iph->daddr;
	    //還沒從network byte order轉成host byte order
	    //因此192.168.50.22目前會是22.50.168.192
	    bpf_printk("Drop ICMP packet sent from %d.%d.%d\n",
			    (key.src) & 0xFF, //192 (第一個byte)
			    (key.src >> 8) & 0xFF, //168
			    (key.src >> 16) & 0xFF); //50
	    //最多四個參數, 所以第四個參數要另外寫
	    bpf_printk(".%d\n", key.src >> 24 & 0xFF); //22

	    long bytes = (data_end - data_start);
	    VALUE *value;
	    VALUE new_value = {0, 0}; //準備一個給第一次的value
	    value = bpf_map_lookup_elem(&drop_icmp_map, &key);
	    if (value == NULL) { //如果這個key還沒有存過任何value
	        //bpf_printk("value is null at first time\n");
		new_value.drop_bytes = bytes;
		new_value.drop_counts = 1;
		bpf_map_update_elem(&drop_icmp_map, &key, &new_value, 0); //0就是BPF_ANY, 存在就更新，不存在就新增
	    } else {
		value->drop_bytes += bytes;
		value->drop_counts++;
		//__sync_fetch_and_add((long *)value->drop_counts, 1); //為了避免更新時userspace也有program在存取map，最好用__sync_fetch_and_add來做in-place更新
		bpf_printk("Drop #%ld ICMP packet, total drop %ld bytes\n", value->drop_counts, value->drop_bytes);
	    }
	    return XDP_DROP;
	}
    }
    return XDP_PASS;
}

char _license[] SEC("license") = "GPL";
