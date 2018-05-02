/* SPDX-License-Identifier: GPL-2.0 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>

#include "kselftest.h"

static int writer_test(int *i)
{
	const struct test {
		const char	*name;
		const char	*dev_name;
		int		flags;
		mode_t		mode;
		long		sleep_sec;
		long		sleep_usec;
		size_t		data_size;
	} tests[] = {
		{
			.name		= "write ready on write only fd",
			.dev_name	= "/dev/scullp0",
			.flags		= O_WRONLY,
			.mode		= S_IWUSR,
			.sleep_sec	= 1,
			.sleep_usec	= 0,
			.data_size	= 0,
		},
		{
			.name		= "write ready on read-write fd",
			.dev_name	= "/dev/scullp1",
			.flags		= O_RDWR,
			.mode		= S_IWUSR,
			.sleep_sec	= 1,
			.sleep_usec	= 0,
			.data_size	= 0,
		},
		{
			.name		= "write 1 byte on write only fd",
			.dev_name	= "/dev/scullp0",
			.flags		= O_WRONLY,
			.mode		= S_IWUSR,
			.sleep_sec	= 1,
			.sleep_usec	= 0,
			.data_size	= 1,
		},
		{
			.name		= "write 1 byte on read-write fd",
			.dev_name	= "/dev/scullp1",
			.flags		= O_RDWR,
			.mode		= S_IWUSR,
			.sleep_sec	= 1,
			.sleep_usec	= 0,
			.data_size	= 1,
		},
		{
			.name		= "write 1024 bytes on write only fd",
			.dev_name	= "/dev/scullp0",
			.flags		= O_WRONLY,
			.mode		= S_IWUSR,
			.sleep_sec	= 1,
			.sleep_usec	= 0,
			.data_size	= 1024,
		},
		{
			.name		= "write 1024 bytes on read-write fd",
			.dev_name	= "/dev/scullp1",
			.flags		= O_RDWR,
			.mode		= S_IWUSR,
			.sleep_sec	= 1,
			.sleep_usec	= 0,
			.data_size	= 1024,
		},
		{ /* sentry */ },
	};
	const char buf[BUFSIZ];
	const struct test *t;
	int fd;

	for (t = tests; t->name; t++) {
		struct timeval tv;
		int maxfd;
		fd_set fds;
		int ret;

		printf("%2d) %-48s: ", (*i)++, t->name);

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

			if (!t->data_size)
				break; /* nothing to write */

			errno = 0;
			ret = write(fd, buf, t->data_size);
			if (ret == -1) {
				perror("write");
				goto fail;
			}
			if (errno) {
				perror("write");
				goto fail;
			}
			if (ret != t->data_size) {
				puts("can't write full data");
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

static int reader_test(int *i)
{
	const struct test {
		const char	*name;
		const char	*dev_name;
		int		flags;
		mode_t		mode;
		long		sleep_sec;
		long		sleep_usec;
		size_t		data_size;
	} tests[] = {
		{
			.name		= "read 1 byte of data",
			.dev_name	= "/dev/scullp1",
			.flags		= O_RDWR,
			.mode		= S_IRUSR|S_IWUSR,
			.sleep_sec	= 1,
			.sleep_usec	= 0,
			.data_size	= 1,
		},
		{
			.name		= "read 1024 bytes of data",
			.dev_name	= "/dev/scullp1",
			.flags		= O_RDWR,
			.mode		= S_IRUSR|S_IWUSR,
			.sleep_sec	= 1,
			.sleep_usec	= 0,
			.data_size	= 1024,
		},
		{ /* sentry */ },
	};
	const struct test *t;
	char buf[BUFSIZ];
	int fd;

	for (t = tests; t->name; t++) {
		struct timeval tv;
		int maxfd;
		fd_set fds;
		int ret;

		printf("%2d) %-48s: ", (*i)++, t->name);

		/* initialize the file descriptors. */
		fd = maxfd = -1;

		fd = open(t->dev_name, t->flags, t->mode);
		if (fd == -1) {
			perror("open");
			goto fail;
		}

		/* write dummy data */
		ret = write(fd, buf, t->data_size);
		if (ret != t->data_size) {
			perror("write");
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
			ret = select(maxfd, &fds, NULL, NULL, &tv);
			if (ret == -1) {
				perror("select");
				goto fail;
			}

			if (!FD_ISSET(fd, &fds)) {
				puts("read is not ready");
				goto fail;
			}

			errno = 0;
			ret = read(fd, buf, t->data_size);
			if (ret == -1) {
				perror("read");
				goto fail;
			}
			if (errno) {
				perror("read");
				goto fail;
			}
			if (ret != t->data_size) {
				puts("can't read full data");
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
	int i = 1;

	if (writer_test(&i))
		ksft_exit_fail();
	if (reader_test(&i))
		ksft_exit_fail();

	ksft_exit_pass();
}
