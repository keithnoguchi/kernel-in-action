# SPDX-License-Identifier: GPL-2.0
KERNDIR ?= /lib/modules/$(shell uname -r)/build
TEST_GEN_PROGS := scullcm_sysfs_test
TEST_GEN_PROGS += scullcm_procfs_test
TEST_GEN_PROGS += scullcm_devfs_test
include $(KERNDIR)/tools/testing/selftests/lib.mk
