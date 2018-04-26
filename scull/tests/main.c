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
	err = ioctl(fd, SCULL_IOCRESET);
	if (err == -1) {
		cmd = "ioctl(SCULL_IOCRESET)";
		goto out;
	}
	quantum = 10;
	err = ioctl(fd, SCULL_IOCSQUANTUM, &quantum);
	if (err == -1) {
		cmd = "ioctl(SCULL_IOCSQUANTUM)";
		goto out;
	}
	err = ioctl(fd, SCULL_IOCTQUANTUM, quantum);
	if (err == -1) {
		cmd = "ioctl(SCULL_IOCTQUANTUM)";
		goto out;
	}
	quantum = ioctl(fd, SCULL_IOCHQSET, quantum);
	if (quantum == -1) {
		err = quantum;
		cmd = "ioctl(SCULL_IOCHQSET)";
		goto out;
	}
out:
	/* reset and close the device before exiting */
	if (ioctl(fd, SCULL_IOCRESET))
		perror("ioctl(SCULL_IOCRESET)");
	if (close(fd))
		perror("close()");

	/* report ioctl() command, in case of error */
	if (err < 0) {
		perror(cmd);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
