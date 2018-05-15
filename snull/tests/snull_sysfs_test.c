/* SPDX-License-Identifier: GPL-2.0 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/if.h>

#include "kselftest.h"

static int sysfs_open_test(int *i)
{
	const struct test {
		const char	*name;
		const char	*file_name;
		int		flags;
	} tests[] = {
		{
			.name = "/sys/module/snull/parameters/tx_lockup file",
			.file_name = "/sys/module/snull/parameters/tx_lockup",
			.flags = O_RDWR,
		},
		{
			.name = "/sys/devices/virtual/net/sn0/ifindex file",
			.file_name = "/sys/devices/virtual/net/sn0/ifindex",
			.flags = O_RDONLY,
		},
		{
			.name = "/sys/devices/virtual/net/sn1/ifindex file",
			.file_name = "/sys/devices/virtual/net/sn1/ifindex",
			.flags = O_RDONLY,
		},
		{
			.name = "/sys/devices/virtual/net/sn0/mtu file",
			.file_name = "/sys/devices/virtual/net/sn0/mtu",
			.flags = O_RDWR,
		},
		{
			.name = "/sys/devices/virtual/net/sn1/mtu file",
			.file_name = "/sys/devices/virtual/net/sn1/mtu",
			.flags = O_RDWR,
		},
		{
			.name = "/sys/devices/virtual/net/sn0/flags file",
			.file_name = "/sys/devices/virtual/net/sn0/flags",
			.flags = O_RDWR,
		},
		{
			.name = "/sys/devices/virtual/net/sn1/flags file",
			.file_name = "/sys/devices/virtual/net/sn1/flags",
			.flags = O_RDWR,
		},
		{ /* sentry */ },
	};
	const struct test *t;
	int fail = 0;

	for (t = tests; t->name; t++) {
		int fd;

		printf("%3d) %-70s", ++(*i), t->name);

		fd = open(t->file_name, t->flags);
		if (fd == -1)
			goto fail;

		if (close(fd)) {
			fd = -1;
			goto fail;
		}
		ksft_inc_pass_cnt();
		puts("PASS");
		continue;
fail:
		if (fd != -1)
			close(fd);
		ksft_inc_fail_cnt();
		puts("FAIL");
		fail++;
	}
	return fail;
}

static int sysfs_flags_test(int *i)
{
	const struct test {
		const char	*name;
		const char	*dev_name;
		long		flags;
		long		want;
	} tests[] = {
		{
			.name		= "sn0 IFF_NOARP check",
			.dev_name	= "sn0",
			.flags		= IFF_NOARP,
			.want		= IFF_NOARP,
		},
		{
			.name		= "sn1 IFF_NOARP check",
			.dev_name	= "sn1",
			.flags		= IFF_NOARP,
			.want		= IFF_NOARP,
		},
		{ /* sentry */ },
	};
	const char *path = "/sys/devices/virtual/net";
	const struct test *t;
	int fail = 0;

	for (t = tests; t->name; t++) {
		char file[BUFSIZ];
		char buf[BUFSIZ];
		int fd = -1;
		long flags;
		int err;

		printf("%3d) %-70s", ++(*i), t->name);

		err = snprintf(file, BUFSIZ, "%s/%s/flags", path, t->dev_name);
		if (err == -1)
			goto fail;
		fd = open(file, O_RDWR);
		if (fd == -1)
			goto fail;
		err = read(fd, buf, BUFSIZ);
		if (err <= 0)
			goto fail;
		buf[err] = '\0';
		flags = strtol(buf, NULL, 0);
		if (flags == -1)
			goto fail;
		if ((flags & t->flags) != t->want)
			goto fail;
		if (close(fd)) {
			fd = -1;
			goto fail;
		}
		ksft_inc_pass_cnt();
		puts("PASS");
		continue;
fail:
		ksft_inc_fail_cnt();
		puts("FAIL");
		fail++;
		if (fd != -1)
			close(fd);
		fd = -1;
	}
	return fail;
}

int main(void)
{
	int fail = 0;
	int i = 0;

	if (sysfs_open_test(&i))
		fail++;
	if (sysfs_flags_test(&i))
		fail++;

	puts("");
	if (fail)
		ksft_exit_fail();
	ksft_exit_pass();
}
