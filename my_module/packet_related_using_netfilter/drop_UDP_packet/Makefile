PWD := $(shell pwd)
KVER := $(shell uname -r)
KDIR = /usr/src/linux-headers-$(KVER)/

MODULE_NAME = drop_UDP
obj-m := $(MODULE_NAME).o

all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean
