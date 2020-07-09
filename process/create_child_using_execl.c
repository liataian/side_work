#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
//On linux need below two
#include <sys/types.h>
#include <sys/wait.h>

//execl will success
#define IFCONFIG_CMD_PATH "/sbin/ifconfig"

//execl will fail intentionally
#define IFCONFIG_WRONG_CMD_PATH "/sbin/ifconfigasd"

//Check if table is valid
int is_valid_interface(const char *iface_str) {
    pid_t pid = fork(); //Create a child process
    int status;

    if (pid < 0) {
	printf("fork failed (%s)", strerror(errno));
	return -1;
    } else if (pid == 0) { //child (return value from fork() is 0)
	if (execl(IFCONFIG_CMD_PATH, "ifconfig", iface_str, (char *) NULL)) { //如果execl執行成功則永遠不會return
	    printf("execl failed (%s)\n", strerror(errno));
	    exit(5); //return status from child
	}
	//所以不用判斷execl成功的狀態
    } else { //parent (return value from fork() is parent’s pid)
        wait(&status); //parent先暫停，等待child return後設定其return status (wait()本身會return child pid)
	printf("WIFEXITED(status)=%d, WEXITSTATUS(status)=%d\n", WIFEXITED(status), WEXITSTATUS(status));
	if (WIFEXITED(status) == 0) { //WIFEXITED不為0為正常退出
		printf("Wrong terminate...\n");
		return 0;
	}
	
	if (WEXITSTATUS(status) != 0) { //WEXITSTATUS為0為正常回傳值
		printf("Invalid interface\n");
		return 0;
	}
    }
    return 1;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Invalid arg number\n");
		return 0;
	}
	const char *iface_str = argv[1]; //en0
	if (is_valid_interface(iface_str)) {
		printf("%s is valid\n", iface_str);
	}
        sleep(10);
	return 0;
}

