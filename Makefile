# SPDX-License-Identifier: GPL-2.0
obj-m += ps/
obj-m += hello/
obj-m += scull/
obj-m += sleepy/
obj-m += scullp/

KERNDIR ?= /lib/modules/$(shell uname -r)/build

all default: modules
install: modules_install
modules modules_install help clean:
	$(MAKE) -C $(KERNDIR) M=$(shell pwd) $@

TARGETS = scull
TARGETS += scullp
run_tests check: kselftest
kselftest:
	for TARGET in $(TARGETS); do                                  \
		$(MAKE) OUTPUT=$(shell pwd)/$$TARGET/tests            \
			CFLAGS="-I$(KERNDIR)/tools/testing/selftests" \
			-C ./$$TARGET/tests/ run_tests;               \
	done
kselftest-clean:
	for TARGET in $(TARGETS); do                                  \
		$(MAKE) OUTPUT=$(shell pwd)/$$TARGET/tests            \
			CFLAGS="-I$(KERNDIR)/tools/testing/selftests" \
			-C ./$$TARGET/tests/ clean;                   \
	done
