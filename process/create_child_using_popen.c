#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
//On linux need below two
#include <sys/types.h>
#include <sys/wait.h>

int is_valid_interface(const char *iface_str) {
    FILE *ptr = NULL;
    char buf[1024];
    int res = 0;
    char cmd[128] = "ip route show dev ";
    strcat(cmd, iface_str);
    printf("cmd=%s\n", cmd);

    if((ptr = popen(cmd, "r")) != NULL) {
        printf("popen run PASS\n");

	while (fgets(buf, sizeof(buf), ptr) != NULL) { //popen成功執行cmd後顯示的訊息(遇到terminator就會換行)
		printf("buf=%s\n", buf);
	}

        int res = pclose(ptr);
	printf("Return value of pclose is: WIFEXITED(res)=%d, WEXITSTATUS(res)=%d, res=%d\n", WIFEXITED(res), WEXITSTATUS(res), res);
	return 1;
    } else {
        printf("Fail to run popen\n");
    }
    return 0;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Invalid arg number\n");
		return 0;
	}
	const char *iface_str = argv[1]; //interface name
	if (is_valid_interface(iface_str)) {
		printf("%s is valid\n", iface_str);
	}
	return 0;
}

