obj-m += usb_key_checker.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all: module app

module:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

app:
	gcc usb_checker.c -o usb_checker

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f usb_checker

