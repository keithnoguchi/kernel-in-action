obj-m += ps/
obj-m += khellod/

KERNDIR ?= /lib/modules/$(shell uname -r)/build

all default: modules
install: modules_install
modules modules_install help clean:
	$(MAKE) -C $(KERNDIR) M=$(shell pwd) $@
