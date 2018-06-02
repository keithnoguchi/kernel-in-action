/* SPDX-License-Identifier: GPL-2.0 */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "kselftest.h"

static int test_sysfs(int *i)
{
	const struct test {
		const char	*name;
		const char	*filename;
		const char	*want;
	} tests[] = {
		{
			.name		= "/sys/module/scullpm/initstate file",
			.filename	= "/sys/module/scullpm/initstate",
			.want		= "live",
		},
		{ /* sentinel */ },
	};
	const struct test *t;
	int fail = 0;

	for (t = tests; t->name; t++) {
		char buf[BUFSIZ];
		char *nl;
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
			printf("FAIL: open(%s): %s\n", t->filename,
			       strerror(errno));
			goto fail_close;
		}
		nl = strchr(buf, '\n');
		if (nl)
			*nl = '\0';
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

	if (test_sysfs(&i))
		fail++;

	if (fail)
		ksft_exit_fail();
	ksft_exit_pass();
}
