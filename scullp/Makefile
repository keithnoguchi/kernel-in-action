# SPDX-License-Identifier: GPL-2.0
obj-m += scullp.o
scullp-objs = main.o

KERNDIR ?= /lib/modules/$(shell uname -r)/build

all default: modules
install: modules_install
modules modules_install help clean:
	$(MAKE) -C $(KERNDIR) M=$(shell pwd) $@
