/* SPDX-License-Identifier: GPL-2.0 */

#include <stdio.h>
#include <stdlib.h>
#include <stropts.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

#include "../scull.h"

static int test_scull_ioctl_quantum(const char *name, const char *scull)
{
	const char *cmd;
	int set_quantum, get_quantum;
	int err;
	int fd;

	printf("%s\n", name);

	/* open scull device. */
	fd = open(scull, O_RDONLY);
	if (fd == -1) {
		perror("open");
		return EXIT_FAILURE;
	}

	/* reset the value first before the test */
	err = ioctl(fd, SCULL_IOCRESET);
	if (err == -1) {
		cmd = "ioctl(SCULL_IOCRESET)";
		goto out;
	}

	/* set and get quantum */
	set_quantum = 10;
	err = ioctl(fd, SCULL_IOCSQUANTUM, &set_quantum);
	if (err == -1) {
		cmd = "ioctl(SCULL_IOCSQUANTUM)";
		goto out;
	}
	err = ioctl(fd, SCULL_IOCGQUANTUM, &get_quantum);
	if (err == -1) {
		cmd = "ioctl(SCULL_IOCGQUANTUM)";
		goto out;
	}
	assert(get_quantum == set_quantum);

	/* tell and query quantum */
	set_quantum = 11;
	err = ioctl(fd, SCULL_IOCTQUANTUM, set_quantum);
	if (err == -1) {
		cmd = "ioctl(SCULL_IOCTQUANTUM)";
		goto out;
	}
	get_quantum = ioctl(fd, SCULL_IOCQQUANTUM);
	if (get_quantum == -1) {
		err = get_quantum;
		cmd = "ioctl(SCULL_IOCQQUANTUM)";
		goto out;
	}
	assert(get_quantum == set_quantum);

	/* exchange quantum */
	set_quantum = 1000;
	get_quantum = set_quantum;
	err = ioctl(fd, SCULL_IOCXQUANTUM, &get_quantum);
	if (err == -1) {
		cmd = "ioctl(SCULL_IOCXQUANTUM)";
		goto out;
	}
	assert(get_quantum == set_quantum);

	/* shift quantum */
	set_quantum = 2;
	get_quantum = ioctl(fd, SCULL_IOCHQUANTUM, set_quantum);
	if (get_quantum == -1) {
		err = get_quantum;
		cmd = "ioctl(SCULL_IOCHQUANTUM)";
		goto out;
	}
	assert(get_quantum == set_quantum);
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

int main(void)
{
	test_scull_ioctl_quantum("1. ioctl(SCULL_IOC?QUANTUM)", "/dev/scull0");
}
