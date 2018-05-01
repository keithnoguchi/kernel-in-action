/* SPDX-License-Identifier: GPL-2.0 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>

#include "kselftest.h"

static int writer_test(void)
{
	const struct test {
		const char	*name;
		const char	*dev_name;
		int		flags;
		mode_t		mode;
		long		sleep_sec;
		long		sleep_usec;
	} tests[] = {
		{
			.name		= "writer ready on write only fd",
			.dev_name	= "/dev/scullp0",
			.flags		= O_WRONLY,
			.mode		= S_IWUSR,
			.sleep_sec	= 1,
			.sleep_usec	= 0,
		},
		{ /* sentry */ },
	};
	const struct test *t;
	int fd;
	int i;

	i = 1;
	for (t = tests; t->name; t++) {
		struct timeval tv;
		int maxfd;
		fd_set fds;
		int ret;

		printf("%2d) %-32s: ", i++, t->name);

		/* initialize the file descriptors. */
		fd = maxfd = -1;

		fd = open(t->dev_name, t->flags, t->mode);
		if (fd == -1) {
			perror("open");
			goto fail;
		}

		/* select(2) based loop */
		maxfd = fd + 1;
		while (1) {
			/* select(2) requires resetting the values on every call */
			tv.tv_sec = t->sleep_sec;
			tv.tv_usec = t->sleep_usec;
			FD_ZERO(&fds);
			FD_SET(fd, &fds);
			ret = select(maxfd, NULL, &fds, NULL, &tv);
			if (ret == -1) {
				perror("select");
				goto fail;
			}

			if (!FD_ISSET(fd, &fds)) {
				puts("write is not ready");
				goto fail;
			}
			break;
		}
		if (close(fd)) {
			fd = -1;
			goto fail;
		}
		ksft_inc_pass_cnt();
		puts("PASS");
	}
	return 0;

fail:
	if (fd != -1)
		close(fd);
	ksft_inc_fail_cnt();
	puts("FAIL");
	return 1;
}

int main(void)
{
	if (writer_test())
		ksft_exit_fail();
	ksft_exit_pass();
}
