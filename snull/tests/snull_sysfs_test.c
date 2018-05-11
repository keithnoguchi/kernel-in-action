/* SPDX-License-Identifier: GPL-2.0 */

#include <stdio.h>

#include "kselftest.h"

static int sysfs_test(int *i)
{
	ksft_inc_fail_cnt();
	return 1;
}

int main(void)
{
	int fail = 0;
	int i = 1;

	if (sysfs_test(&i))
		fail++;

	puts("");
	if (fail)
		ksft_exit_fail();
	ksft_exit_pass();
}
