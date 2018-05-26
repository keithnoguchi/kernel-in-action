# SPDX-License-Identifier: GPL-2.0
obj-m += ls/
obj-m += lspci/
obj-m += hello/
obj-m += scull/
obj-m += sleepy/
obj-m += scullp/
obj-m += scullc/
obj-m += ldd/
obj-m += sculld/
obj-m += currenttime/
obj-m += snull/

KERNDIR ?= /lib/modules/$(shell uname -r)/build

all default: modules
install: modules_install
modules modules_install help clean:
	$(MAKE) -C $(KERNDIR) M=$(shell pwd) $@

.PHONY: load unload reload
load: unload modules_install
	@for MODULE in $(patsubst %/,%,$(obj-m)); do modprobe $$MODULE; done
unload:
	-@for MODULE in $(patsubst %/,%,$(obj-m)); do modprobe -r $$MODULE; done
reload: load

TARGETS = scull
TARGETS += scullp
TARGETS += sleepy
TARGETS += ldd
TARGETS += sculld
TARGETS += currenttime
TARGETS += lspci
TARGETS += snull

.PHONY: run_tests check kselftest kselftest-clean
run_tests check: kselftest
kselftest: reload
	@for TARGET in $(TARGETS); do                                 \
		$(MAKE) OUTPUT=$(shell pwd)/$$TARGET/tests           \
			CFLAGS="-I$(KERNDIR)/tools/testing/selftests" \
			-C ./$$TARGET/tests/ run_tests;               \
	done
kselftest-clean:
	@for TARGET in $(TARGETS); do                                 \
		$(MAKE) OUTPUT=$(shell pwd)/$$TARGET/tests           \
			CFLAGS="-I$(KERNDIR)/tools/testing/selftests" \
			-C ./$$TARGET/tests/ clean;                   \
	done
