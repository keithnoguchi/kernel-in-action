/* SPDX-License-Identifier: GPL-2.0 */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "kselftest.h"

static int test_devfs(int *i)
{
	const struct test {
		const char	*name;
		const char	*devname;
		size_t		writen;
	} tests[] = {
		{
			.name		= "/dev/scullcm0 simple open",
			.devname	= "/dev/scullcm0",
		},
		{
			.name		= "/dev/scullcm1 simple open",
			.devname	= "/dev/scullcm1",
		},
		{
			.name		= "/dev/scullcm2 simple open",
			.devname	= "/dev/scullcm2",
		},
		{
			.name		= "/dev/scullcm3 simple open",
			.devname	= "/dev/scullcm3",
		},
		{
			.name		= "/dev/scullcm0 1024 bytes write",
			.devname	= "/dev/scullcm0",
			.writen		= 1024,
		},
		{
			.name		= "/dev/scullcm1 1024 bytes write",
			.devname	= "/dev/scullcm1",
			.writen		= 1024,
		},
		{
			.name		= "/dev/scullcm2 1024 bytes write",
			.devname	= "/dev/scullcm2",
			.writen		= 1024,
		},
		{
			.name		= "/dev/scullcm3 1024 bytes write",
			.devname	= "/dev/scullcm3",
			.writen		= 1024,
		},
		{
			.name		= "/dev/scullcm0 4096 bytes write",
			.devname	= "/dev/scullcm0",
			.writen		= 4096,
		},
		{
			.name		= "/dev/scullcm1 4096 bytes write",
			.devname	= "/dev/scullcm1",
			.writen		= 4096,
		},
		{
			.name		= "/dev/scullcm2 4096 bytes write",
			.devname	= "/dev/scullcm2",
			.writen		= 4096,
		},
		{
			.name		= "/dev/scullcm3 4096 bytes write",
			.devname	= "/dev/scullcm3",
			.writen		= 4096,
		},
		{ /* sentry */ },
	};
	const struct test *t;
	int fail = 0;

	for (t = tests; t->name; t++) {
		int err;
		int fd;

		printf("%3d) %-12s: %-55s", ++(*i), __FUNCTION__, t->name);

		fd = open(t->devname, O_RDWR);
		if (fd == -1) {
			printf("FAIL: open(%s): %s\n", t->devname, strerror(errno));
			goto fail;
		}
		if (t->writen) {
			char *buf = malloc(t->writen);
			int ret = write(fd, buf, t->writen);
			free(buf);
			if (ret == -1) {
				printf("FAIL: write(%d): %s\n",
				       t->writen, strerror(errno));
				goto fail_close;
			}
			if (ret != t->writen) {
				printf("FAIL: %d=write(%d)\n", ret, t->writen);
				goto fail_close;
			}
		}
		close(fd);
		puts("PASS");
		ksft_inc_pass_cnt();
		continue;
fail_close:
		close(fd);
fail:
		ksft_inc_fail_cnt();
		fail++;
	}
	return fail;
}

int main(void)
{
	int fail = 0;
	int i = 0;

	if (test_devfs(&i))
		fail++;

	if (fail)
		ksft_exit_fail();
	ksft_exit_pass();
}
