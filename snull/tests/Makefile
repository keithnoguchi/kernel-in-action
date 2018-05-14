# SPDX-License-Identifier: GPL-2.0
KERNDIR ?= /lib/modules/$(shell uname -r)/build
CFLAGS += -I$(KERNDIR)/tools/testing/selftests
OUTPUT ?= $(shell pwd)
TEST_GEN_PROGS := snull_sysfs_test
include $(KERNDIR)/tools/testing/selftests/lib.mk
