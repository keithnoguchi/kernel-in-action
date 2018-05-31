/* SPDX-License-Identifier: GPL-2.0 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "kselftest.h"

static int sysfs_test(int *i)
{
	return 0;
}

int main(void)
{
	int fail = 0;
	int i = 0;

	if (sysfs_test(&i))
		fail++;

	if (fail)
		ksft_exit_fail();
	else
		ksft_exit_pass();
}
