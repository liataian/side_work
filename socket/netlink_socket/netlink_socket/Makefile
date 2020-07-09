PWD := $(shell pwd)
KERNEL_VERSION := $(shell uname -r)

#KERNEL_DIR = /usr/src/linux-headers-$(KERNEL_VERSION)/
KERNEL_DIR = /lib/modules/$(shell uname -r)/build/
KERNEL_FINAL_DIR := $(KERNEL_DIR)
#KERNEL_DIR = /lib/modules/

MODULE_NAME = netlink_kernel
obj-m := $(MODULE_NAME).o

all:
	@echo $(KERNEL_FINAL_DIR) && make -C $(KERNEL_FINAL_DIR) M=$(PWD) modules
clean:
	make -C $(KERNEL_FINAL_DIR) M=$(PWD) clean
