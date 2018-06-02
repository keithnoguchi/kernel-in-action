/* SPDX-License-Identifier: GPL-2.0 */
#include <stdio.h>
#include <errno.h>
#include <string.h>
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
		const char	*want;
	} tests[] = {
		{
			.name		= "check scullcm major:minor exists in /proc/devices",
			.filename	= "/proc/devices",
			.want		= " scullpm\n",
		},
		{ /* sentinel */ },
	};
	const struct test *t;
	int fail = 0;

	for (t = tests; t->name; t++) {
		char buf[BUFSIZ];
		int ret;
		int fd;

		printf("%3d) %-12s: %-55s", ++(*i), __FUNCTION__, t->name);

		fd = open(t->filename, O_RDONLY);
		if (fd == -1) {
			printf("FAIL: open(%s): %s\n", t->filename,
			       strerror(errno));
			goto fail;
		}
		ret = read(fd, buf, sizeof(buf));
		if (ret == -1) {
			printf("FAIL: read(%s): %s\n", t->filename,
			       strerror(errno));
			goto fail_close;
		}
		if (!strstr(buf, t->want)) {
			printf("FAIL: strstr(%s)\n", t->want);
			goto fail_close;
		}
		close(fd);
		ksft_inc_pass_cnt();
		puts("PASS");
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

	if (test_procfs(&i))
		fail++;

	if (fail)
		ksft_exit_fail();
	ksft_exit_pass();
}
