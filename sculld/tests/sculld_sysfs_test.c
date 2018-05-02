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
		const char	*want;
	} tests[] = {
		{
			.name      = "/sys/bus/ldd/drivers/sculld/version test",
			.file_name = "/sys/bus/ldd/drivers/sculld/version",
			.want      = "1.0",
		},
		{ /* sentry */ },
	};
	const struct test *t;
	int flags = O_RDONLY;
	int fd = -1;

	for (t = tests; t->name; t++) {
		printf("%2d) %-70s", (*i)++, t->name);
		char buf[BUFSIZ];
		const char *got;
		int err;

		fd = open(t->file_name, flags);
		if (fd == -1) {
			perror("open");
			goto fail;
		}

		err = read(fd, buf, BUFSIZ);
		if (err == -1) {
			perror("read");
			goto fail;
		}
		buf[err-1] = '\0'; /* -1: strip new line */

		got = buf;
		if (strcmp(got, t->want)) {
			printf("FAIL, want=%s, got=%s\n", t->want, got);
			goto fail;
		}
		if (close(fd)) {
			perror("close");
			goto fail;
		}
		puts("PASS");
	}
	ksft_inc_pass_cnt();
	return 0;
fail:
	puts("FAIL");
	ksft_inc_fail_cnt();
	if (fd != -1)
		close(fd);
	return 1;
}

int main(void)
{
	int i;

	i = 1;
	if (sysfs_test(&i))
		ksft_exit_fail();
	ksft_exit_pass();
}
