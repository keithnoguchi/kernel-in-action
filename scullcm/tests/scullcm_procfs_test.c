/* SPDX-License-Identifier: GPL-2.0 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "kselftest.h"

static int test_procfs(int *i)
{
	const struct test {
		const char	*name;
		const char	*filename;
		const char	*needle;
	} tests[] = {
		{
			.name		= "find scullcm in /proc/devices",
			.filename	= "/proc/devices",
			.needle		= "scullcm",
		},
		{
			.name		= "find scullcm_qset in /proc/slabinfo",
			.filename	= "/proc/slabinfo",
			.needle		= "scullcm_qset",
		},
		{ /* sentinel */ },
	};
	const struct test *t;
	int fail = 0;

	for (t = tests; t->name; t++) {
		char buf[BUFSIZ];
		char *find;
		int err;
		int fd;

		printf("%3d) %-12s: %-55s", ++(*i), __FUNCTION__, t->name);
		fd = open(t->filename, O_RDONLY);
		if (fd == -1) {
			printf("FAIL: open(): %s\n", t->filename, strerror(errno));
			goto fail;
		}
		err = read(fd, buf, sizeof(buf));
		if (err == -1) {
			printf("FAIL: read(): %s\n", t->filename, strerror(errno));
			goto close_fail;
		}
		buf[err] = '\0';
		find = strstr(buf, t->needle);
		if (!find) {
			printf("FAIL: strcstr(%s, %s)\n", buf, t->needle);
			goto close_fail;
		}
		close(fd);
		puts("PASS");
		ksft_inc_pass_cnt();
		continue;
close_fail:
		close(fd);
fail:
		fail++;
		ksft_inc_fail_cnt();
	}

	return fail;
}

int main(void)
{
	int fail = 0;
	int i = 0;

	if (test_procfs(&i))
		fail++;

	if (fail)
		ksft_exit_fail();
	else
		ksft_exit_pass();
}
