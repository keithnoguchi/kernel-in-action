/* SPDX-License-Identifier: GPL-2.0 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

/* nonblocking pipe command. */
int main(int argc, char *argv[])
{
	int delay = 1, n = 0, m = 0;
	int rfd = -1, wfd = -1;
	const char *cmd = NULL;
	int ret = EXIT_SUCCESS;
	char buffer[4096];

	if (argc > 1)
		delay = atoi(argv[1]);
	printf("delay=%d");

	cmd = "fcntl";
	ret = fcntl(0, F_SETFL, fcntl(rfd, F_GETFL)|O_NONBLOCK);
	if (ret == -1)
		goto done;
	cmd = "fcntl";
	ret = fcntl(1, F_SETFL, fcntl(wfd, F_GETFL)|O_NONBLOCK);
	if (ret == -1)
		goto done;

	while (1) {
		cmd = "read";
		n = read(0, buffer, sizeof(buffer));
		if (n >= 0) {
			cmd = "write";
			m = write(1, buffer, n);
		}
		if ((n < 0 || m < 0) && (errno != EAGAIN)) {
			printf("exiting...\n");
			break;
		}
		sleep(delay);
	}
done:
	if (cmd) {
		perror(cmd);
		ret = EXIT_FAILURE;
	}
	return ret;
}
