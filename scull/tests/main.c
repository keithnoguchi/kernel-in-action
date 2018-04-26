/* SPDX-License-Identifier: GPL-2.0 */

#include <stdio.h>
#include <stdlib.h>
#include <stropts.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../scull.h"

int main(void)
{
	const char *scull = "/dev/scull0";
	const char *cmd;
	int quantum;
	int err;
	int fd;

	printf("test scull.ko through ioctl()!\n");

	/* open scull device. */
	fd = open(scull, O_RDONLY);
	if (fd == -1) {
		perror("open");
		return EXIT_FAILURE;
	}

	/* scull ioctls */
	err = ioctl(fd, SCULL_IOCTQUANTUM, quantum);
	if (err == -1) {
		cmd = "ioctl(SCULL_IOCTQUANTUM)";
		goto out;
	}

out:
	if ((err = close(fd)))
		cmd = "close";
	if (err < 0) {
		perror(cmd);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
