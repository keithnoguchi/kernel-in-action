/* SPDX-License-Identifier: GPL-2.0 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define SCULLP_DEV_NAME		"/dev/scullp0"

/* nonblocking pipe command. */
int main(int argc, char *argv[])
{
	int delay = 1, n = 0, m = 0;
	int rfd = -1, wfd = -1;
	const char *cmd = NULL;
	char buffer[4096];
	int ret;

	if (argc > 1)
		delay = atoi(argv[1]);
	printf("delay=%d");

	cmd = "open";
	rfd = open(SCULLP_DEV_NAME, O_RDONLY);
	if (rfd == -1)
		goto out;
	cmd = "fcntl";
	ret = fcntl(rfd, F_SETFL, fcntl(rfd, F_GETFL)|O_NONBLOCK);
	if (ret == -1)
		goto out;

	cmd = "open";
	wfd = open(SCULLP_DEV_NAME, O_WRONLY);
	if (wfd == -1)
		goto out;
	cmd = "fcntl";
	ret = fcntl(wfd, F_SETFL, fcntl(wfd, F_GETFL)|O_NONBLOCK);
	if (ret == -1)
		goto out;

	while (1) {
		cmd = "read";
		n = read(rfd, buffer, sizeof(buffer));
		if (n >= 0) {
			cmd = "write";
			m = write(wfd, buffer, n);
		}
		if ((n < 0 || m < 0) && (errno != EAGAIN)) {
			printf("exiting...\n");
			break;
		}
		sleep(delay);
	}
out:
	if (rfd != -1)
		close(rfd);
	if (wfd != -1)
		close(wfd);
	if (cmd) {
		perror(cmd);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
