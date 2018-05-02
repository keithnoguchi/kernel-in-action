/* SPDX-License-Identifier: GPL-2.0 */

#include <stdio.h>

#include "kselftest.h"

static int sysfs_test(int *i)
{
	/* just fail for now */
	return 1;
}

int main(void)
{
	int i;

	i = 1;
	if (sysfs_test(&i))
		ksft_exit_fail();
	ksft_exit_pass();
}
