/* SPDX-License-Identifier: GPL-2.0 */
#include <stdio.h>
#include <stdlib.h>
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
		const char	*sysfsname;
		size_t		writen;
		size_t		readn;
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
			.sysfsname	= "/sys/bus/ldd/devices/scullcm0/size",
			.writen		= 1024,
		},
		{
			.name		= "/dev/scullcm1 1024 bytes write",
			.devname	= "/dev/scullcm1",
			.sysfsname	= "/sys/bus/ldd/devices/scullcm1/size",
			.writen		= 1024,
		},
		{
			.name		= "/dev/scullcm2 1024 bytes write",
			.devname	= "/dev/scullcm2",
			.sysfsname	= "/sys/bus/ldd/devices/scullcm2/size",
			.writen		= 1024,
		},
		{
			.name		= "/dev/scullcm3 1024 bytes write",
			.devname	= "/dev/scullcm3",
			.sysfsname	= "/sys/bus/ldd/devices/scullcm3/size",
			.writen		= 1024,
		},
		{
			.name		= "/dev/scullcm0 4096 bytes write",
			.devname	= "/dev/scullcm0",
			.sysfsname	= "/sys/bus/ldd/devices/scullcm0/size",
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
		{
			.name		= "/dev/scullcm0 1024 bytes write & read",
			.devname	= "/dev/scullcm0",
			.writen		= 1024,
			.readn		= 1024,
		},
		{
			.name		= "/dev/scullcm1 1024 bytes write & read",
			.devname	= "/dev/scullcm1",
			.writen		= 1024,
			.readn		= 1024,
		},
		{
			.name		= "/dev/scullcm2 1024 bytes write & read",
			.devname	= "/dev/scullcm2",
			.writen		= 1024,
			.readn		= 1024,
		},
		{
			.name		= "/dev/scullcm3 1024 bytes write & read",
			.devname	= "/dev/scullcm3",
			.writen		= 1024,
			.readn		= 1024,
		},
		{
			.name		= "/dev/scullcm0 4096 bytes write & read",
			.devname	= "/dev/scullcm0",
			.writen		= 4096,
			.readn		= 4096,
		},
		{
			.name		= "/dev/scullcm1 4096 bytes write & read",
			.devname	= "/dev/scullcm1",
			.writen		= 4096,
			.readn		= 4096,
		},
		{
			.name		= "/dev/scullcm2 4096 bytes write & read",
			.devname	= "/dev/scullcm2",
			.writen		= 4096,
			.readn		= 4096,
		},
		{
			.name		= "/dev/scullcm3 4096 bytes write & read",
			.devname	= "/dev/scullcm3",
			.writen		= 4096,
			.readn		= 4096,
		},
		{
			.name		= "/dev/scullcm0 32768 bytes write & read",
			.devname	= "/dev/scullcm0",
			.writen		= 32768,
			.readn		= 32768,
		},
		{
			.name		= "/dev/scullcm1 32768 bytes write & read",
			.devname	= "/dev/scullcm1",
			.writen		= 32768,
			.readn		= 32768,
		},
		{
			.name		= "/dev/scullcm2 32768 bytes write & read",
			.devname	= "/dev/scullcm2",
			.writen		= 32768,
			.readn		= 32768,
		},
		{
			.name		= "/dev/scullcm3 32768 bytes write & read",
			.devname	= "/dev/scullcm3",
			.writen		= 32768,
			.readn		= 32768,
		},
		{ /* sentry */ },
	};
	const struct test *t;
	int fail = 0;

	for (t = tests; t->name; t++) {
		char *wbuf = NULL, *rbuf = NULL;
		int err;
		int fd;

		printf("%3d) %-12s: %-55s", ++(*i), __FUNCTION__, t->name);

		/* always starts with fresh */
		fd = open(t->devname, O_WRONLY|O_TRUNC);
		if (fd == -1) {
			printf("FAIL: open(%s): %s\n", t->devname, strerror(errno));
			goto fail;
		}
		if (t->writen) {
			int total;
			int ret;

			wbuf = malloc(t->writen);
			memset(wbuf, 'w', t->writen);
			/* multiple writes */
			total = 0;
			while (total < t->writen) {
				ret = write(fd, wbuf+total, t->writen-total);
				if (ret == -1) {
					printf("FAIL: write(%d): %s\n",
					       t->writen, strerror(errno));
					goto fail_free_close;
				}
				if (!ret)
					break;
				total += ret;
			}
			if (total != t->writen) {
				printf("FAIL: %d=write(%d)\n", total, t->writen);
				goto fail_free_close;
			}
			if (t->sysfsname) {
				char buf[BUFSIZ];
				char *nl;
				int got;
				int ret;
				int sfd;

				sfd = open(t->sysfsname, O_RDONLY);
				if (fd == -1) {
					printf("FAIL: open(%s): %s\n",
					       t->sysfsname, strerror(errno));
					goto fail_free_close;
				}
				ret = read(sfd, buf, sizeof(buf));
				if (ret == -1) {
					printf("FAIL: read(%s): %s\n",
					       t->sysfsname, strerror(errno));
					close(sfd);
					goto fail_free_close;
				}
				close(sfd);
				nl = strchr(buf, '\n');
				if (nl)
					*nl = '\0';
				got = atoi(buf);
				if (got != t->writen) {
					printf("FAIL: device file: got=%d, want=%d\n",
					       got, t->writen);
					goto fail_free_close;
				}
			}
			if (t->readn) {
				int total;
				int ret;
				int rfd;
				int i;

				rfd = open(t->devname, O_RDONLY);
				if (rfd == -1) {
					printf("FAIL: open(%s): %s\n", t->devname,
					       strerror(errno));
					goto fail_free_close;
				}
				rbuf = malloc(t->readn);
				memset(rbuf, 'r', t->readn);
				/* read all */
				total = 0;
				while (total < t->readn) {
					ret = read(rfd, rbuf+total, t->readn-total);
					if (ret == -1) {
						printf("FAIL: read(%d): %s\n",
						       t->readn, strerror(errno));
						close(rfd);
						goto fail_free_close;
					}
					if (ret == 0)
						break;
					total += ret;
				}
				close(rfd);
				if (total != t->readn) {
					printf("FAIL: %d=read(%d)\n", total, t->readn);
					goto fail_free_close;
				}
				for (i = 0; i < t->readn; i++)
					if (rbuf[i] != 'w') {
						printf("FAIL: '%c'(rbuf[%d])!='%c'\n",
						       rbuf[i], i, 'w');
						goto fail_free_close;
					}
				free(rbuf);
			}
			free(wbuf);
		}
		close(fd);
		puts("PASS");
		ksft_inc_pass_cnt();
		continue;
fail_free_close:
		if (wbuf)
			free(wbuf);
		if (rbuf)
			free(rbuf);
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
