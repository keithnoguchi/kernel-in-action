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

int main(void)
{
	const char *dev_name = SCULLP_DEV_NAME;
	const char *err_name = NULL;
	int rfd, wfd, maxfd;
	struct timeval tv;
	fd_set rfds;
	int ret;

	printf("select(2) based scullp.ko tests\n");

	/* initialize the file descriptors. */
	rfd = wfd = maxfd = -1;

	err_name = "open(" SCULLP_DEV_NAME ")";
	ret = rfd = open(dev_name, O_RDONLY);
	if (rfd == -1)
		goto out;

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

		err_name = "select";
		ret = select(maxfd, &rfds, NULL, NULL, &tv);
		if (ret == -1) {
			goto out;
		}
		printf("wakeup\n");
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
