/* SPDX-License-Identifier: GPL-2.0 */

#include <stdio.h>
#include <sys/select.h>

#define SELECT_INTERVAL		2	/* wakes up every 2 sec */

int main(void)
{
	struct timeval tv;
	fd_set rfds;
	int ret;

	printf("select(2) based scullp.ko tests\n");

	while (1) {
		tv.tv_sec = SELECT_INTERVAL;
		tv.tv_usec = 0;

		ret = select(1, &rfds, NULL, NULL, &tv);
		if (ret == -1) {
			perror("select");
			break;
		}
		printf("wakeup\n");
	}
	return ret;
}
