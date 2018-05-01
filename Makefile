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

TESTS = scull
check: kselftest
kselftest:
	for d in $(TESTS); do $(Q)$(MAKE) -C ./$${d}/tests/ run_tests; done
kselftest-clean:
	for d in $(TESTS); do $(Q)$(MAKE) -C ./$${d}/tests/ clean; done
