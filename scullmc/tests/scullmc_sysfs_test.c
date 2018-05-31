/* SPDX-License-Identifier: GPL-2.0 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "kselftest.h"

static int sysfs_test(int *i)
{
	const struct test {
		const char	*name;
		const char	*file_name;
		int		flags;
		const char	*want;
	} tests[] = {
		{
			.name		= "/sys/bus/ldd/drivers/scullmc/version version",
			.file_name	= "/sys/bus/ldd/drivers/scullmc/version",
			.want		= "1.3.1",
		},
		{
			.name		= "/sys/bus/ldd/devices/scullmc0/uevent read",
			.file_name	= "/sys/bus/ldd/devices/scullmc0/uevent",
			.flags		= O_RDONLY,
		},
		{
			.name		= "/sys/bus/ldd/devices/scullmc1/uevent read",
			.file_name	= "/sys/bus/ldd/devices/scullmc1/uevent",
			.flags		= O_RDONLY,
		},
		{
			.name		= "/sys/bus/ldd/devices/scullmc2/uevent read",
			.file_name	= "/sys/bus/ldd/devices/scullmc2/uevent",
			.flags		= O_RDONLY,
		},
		{
			.name		= "/sys/bus/ldd/devices/scullmc3/uevent read",
			.file_name	= "/sys/bus/ldd/devices/scullmc3/uevent",
			.flags		= O_RDONLY,
		},
		{ /* sentinel */ },
	};
	const struct test *t;
	int fail = 0;

	for (t = tests; t->name; t++) {
		char buf[1024];
		char *nl;
		int err;
		int fd;

		printf("%3d) %-12s: %-55s", ++(*i), __FUNCTION__, t->name);

		fd = open(t->file_name, t->flags);
		if (fd == -1) {
			puts("FAIL: open()");
			ksft_inc_fail_cnt();
			fail++;
			continue;
		}
		err = read(fd, buf, sizeof(buf));
		if (err == -1) {
			puts("FAIL: read()");
			ksft_inc_fail_cnt();
			fail++;
			goto close;
		}
		nl = strchr(buf, '\n');
		if (nl)
			*nl = '\0';
		if (t->want && strcmp(buf, t->want)) {
			printf("FAIL: got='%s', want='%s'\n", buf, t->want);
			ksft_inc_fail_cnt();
			fail++;
			goto close;
		}
		puts("PASS");
		ksft_inc_pass_cnt();
close:
		close(fd);
	}
	return fail;
}

int main(void)
{
	int fail = 0;
	int i = 0;

	if (sysfs_test(&i))
		fail++;

	if (fail)
		ksft_exit_fail();
	else
		ksft_exit_pass();
}
