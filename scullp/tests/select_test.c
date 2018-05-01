/* SPDX-License-Identifier: GPL-2.0 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>

#define SELECT_INTERVAL		2		/* wakes up every 2 sec */
#define SCULLP_DEV_NAME		"/dev/scullp0"	/* scullp device name */

#ifndef max
#define max(A, B)		((A) > (B) ? (A) : (B))
#endif /* !max */


static int open_test(void)
{
	const struct test {
		const char	*name;
		const char	*dev_name;
		int		flags;
		mode_t		mode;
	} tests[] = {
		{
			.name =		"read only",
			.dev_name =	"/dev/scullp0",
			.flags =	O_RDONLY,
			.mode =		S_IRUSR,
		},
		{
			/* sentry */
		},
	};
	const struct test *t;
	char buf[BUFSIZ];
	int err;
	int i;

	i = 1;
	for (t = tests; t->name != NULL; t++) {
		snprintf(buf, BUFSIZ, "%d: %s", i++, t->name);
		err = open(t->dev_name, t->flags, t->mode);
		if (err == -1) {
			perror(buf);
			return 1;
		}
		printf("%s [OK]\n", buf);
	}
	return 0;
}

static int old_test(void)
{
	const char *dev_name = SCULLP_DEV_NAME;
	const char *err_name = NULL;
	int rfd, wfd, maxfd;
	struct timeval tv;
	fd_set rfds, wfds;
	int ret;

	printf("select(2) based scullp.ko tests\n");

	/* initialize the file descriptors. */
	rfd = wfd = maxfd = -1;

	/* read descriptor */
	err_name = "open(" SCULLP_DEV_NAME ")";
	ret = rfd = open(dev_name, O_RDONLY);
	if (rfd == -1)
		goto out;

	/* write descriptor */
	err_name = "open(" SCULLP_DEV_NAME ")";
	ret = wfd = open(dev_name, O_WRONLY);
	if (wfd == -1)
		goto out;

	/* select(2) based loop */
	maxfd = max(rfd, wfd) + 1;
	while (1) {
		tv.tv_sec = SELECT_INTERVAL;
		tv.tv_usec = 0;

		/* select(2) requires resetting rfds before the call */
		FD_ZERO(&rfds);
		FD_SET(rfd, &rfds);
		FD_ZERO(&wfds);
		FD_SET(wfd, &wfds);

		err_name = "select";
		ret = select(maxfd, &rfds, &wfds, NULL, &tv);
		if (ret == -1) {
			goto out;
		}
		printf("wakeup\n");

		if (FD_ISSET(rfd, &rfds))
			printf("ready to read\n");
		else if (FD_ISSET(wfd, &wfds))
			printf("ready to write\n");
	}

out:
	if (wfd != -1)
		close(wfd);
	if (rfd != -1)
		close(rfd);
	if (ret < 0)
		perror(err_name);

	return ret;
}

int main(void)
{
	if (open_test())
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
