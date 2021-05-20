#include <stdio.h>
#include <stdlib.h>
#include <signal.h> //for signal()
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/bpf.h>
//使用"gcc -v -E -"可得知gcc會從/usr/include/bpf/找到bpf.h, libbpf.h, bpf_endian.h
#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include <bpf/bpf_endian.h>
#include <unistd.h> //for sleep() and access()
#include <net/if.h> //for if_nametoindex()
#include <sys/resource.h> //for "struct rlimit"

const char xdp_prog[] = "xdp_prog.o";
const char prog_path[] = "/sys/fs/bpf/prog_drop_icmp";
const char map_path[] = "/sys/fs/bpf/map_drop_icmp";
unsigned int ifindex = 0;

typedef struct key {
    unsigned int saddr;
    unsigned int daddr;
} KEY;

typedef struct value {
    long drop_bytes;
    long drop_count;
} VALUE;

static void int_exit(int sig) {
    printf("stopping\n");
    bpf_set_link_xdp_fd(ifindex, -1, 0);
    exit(0);
}

//可參考xdp_router_ipv4_user.c
int main(int argc, char **argv) {
    struct bpf_object *obj;
    int prog_fd;
    int map_fd;
    char prog_name[256];
    snprintf(prog_name, sizeof(prog_name), xdp_prog);

    struct bpf_prog_load_attr prog_attr = {
	    .prog_type = BPF_PROG_TYPE_XDP,
	    .file = prog_name
    };

    signal(SIGINT, int_exit);
    signal(SIGTERM, int_exit);

    //先確定XDP program是否已經pin過(永久可存取), 有pin過就重新使用fd
    if (access(prog_path, F_OK) == 0) {
	// "/sys/fs/bpf/prog_drop_icmp" already exists
	prog_fd = bpf_obj_get(prog_path);
	printf("Reuse XDP program fd: %d\n", prog_fd);
    } else {
	//Load XDP program(obj file) to kernel (would creates map also)
	if (bpf_prog_load_xattr(&prog_attr, &obj, &prog_fd)) {
	    printf("Failed to load BPF program\n");
	    return -1;
	}

	if (!prog_fd) {
	    printf("There is no BPF program fd exists\n");
	    return -1;
	}
	printf("Loading XDP program (fd: %d) into kernel done.\n", prog_fd);

	if (bpf_obj_pin(prog_fd, prog_path)) {
	    printf("Program already exists\n");
	}
	printf("Pin XDP program to \'%s\' done\n", prog_path);
    }

    //確認map是否已經pin過(永久可存取), 有pin過就重新使用fd
    if (access(map_path, F_OK) == 0) {
	// "/sys/fs/bpf/map_drop_icmp" already exists
	map_fd = bpf_obj_get(map_path);
	printf("Reuse map fd: %d\n", map_fd);
    } else {
	map_fd = bpf_object__find_map_fd_by_name(obj, "drop_icmp_map");

	if (!map_fd) {
	    printf("There is no BPF map fd exists\n");
	    return -1;
	}
	printf("Get map fd : %d\n", map_fd);

	if (bpf_obj_pin(map_fd, map_path)) {
	    printf("Map already exists\n");
	}
	printf("Pin map to \'%s\' done\n", map_path);
    }

    if (!(ifindex = if_nametoindex("wlan0"))) {
        printf("Failed to get interface index\n");
	return -1;
    }

    //Attach BPF program to XDP hook point
    if (bpf_set_link_xdp_fd(ifindex, prog_fd, 0)) {
         printf("Failed to attach BPF program\n");
	 return -1;
    }

    printf("Attach XDP program done\n");

    while(1) {
	    sleep(3); //每N秒找一次

	    //如果要從頭開始找，可先傳入一個一定不存在的key，nexy_key就會成為map中第一個key
	    KEY current_key = {0, 0}; //init key
	    KEY next_key;
	    VALUE value;
	    printf("Start searching...\n");
	    while(bpf_map_get_next_key(map_fd, &current_key, &next_key) == 0) { //有找到next key
		bpf_map_lookup_elem(map_fd, &next_key, &value); //讀取這個key跟value
		long bytes = value.drop_bytes;
		long count = value.drop_count;
#if 0
		//使用ntoa的寫法(不推薦)
		struct in_addr src = { next_key.saddr }; //直接init
		struct in_addr dst = { next_key.daddr };
		//TODO 為何inet_ntoa寫在同一行輸出會相同?
		//因為inet_ntoa是用static buffer存, 後寫入的結果會蓋掉先寫入的結果, 除非每次call都用strdup或strcpy存起來
		//參考: https://stackoverflow.com/questions/36524765/why-inet-ntoa-called-twice-in-printf-gives-wrong-output
		printf("Found dropped ICMP: src=%s, dst=%s, bytes=%ld, count=%ld\n",
				inet_ntoa(src), inet_ntoa(dst), bytes, count);

		printf("Found dropped ICMP:\n");
		printf("src=%s\n", inet_ntoa(src)); //inet_ntoa已不推薦使用, 建議用inet_ntop (不使用static buffer來存字串)
		printf("dst=%s\n", inet_ntoa(dst));
		printf("bytes=%ld, count=%ld\n", bytes, count);
#else
		//使用ntop的寫法
		char src[INET_ADDRSTRLEN];
		char dst[INET_ADDRSTRLEN];
                printf("src=%s, dst=%s, already dropped %ld packets (%ld bytes)\n",
				inet_ntop(AF_INET, &next_key.saddr, src, INET_ADDRSTRLEN),
				inet_ntop(AF_INET, &next_key.daddr, dst, INET_ADDRSTRLEN),
				count,
				bytes);
#endif
		current_key = next_key;
	    }
	    current_key.saddr = 0;
	    current_key.daddr = 0;
    }

    //Detach XDP program, 不會再過濾ICMP, map也就不會再count了
    bpf_set_link_xdp_fd(ifindex, -1, 0);

    return 0;
}
