# SPDX-License-Identifier: GPL-2.0
obj-m += sculld.o
sculld-objs = main.o

KERNDIR ?= /lib/modules/$(shell uname -r)/build

all: modules
install: modules_install
modules modules_install all default help:
	$(MAKE) -C $(KERNDIR) M=$(shell pwd) $@
