/* SPDX-License-Identifier: GPL-2.0 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "kselftest.h"

static int sysfs_open_test(int *i)
{
	const struct test {
		const char	*name;
		const char	*file_name;
		int		flags;
	} tests[] = {
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

int main(void)
{
	int fail = 0;
	int i = 0;

	if (sysfs_open_test(&i))
		fail++;

	puts("");
	if (fail)
		ksft_exit_fail();
	ksft_exit_pass();
}
