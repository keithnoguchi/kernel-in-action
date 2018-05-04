/* SPDX-License-Identifier: GPL-2.0 */

#include <stdio.h>

#include "kselftest.h"

static int procfs_test(int *i)
{
	ksft_inc_fail_cnt();
	return 1;
}

int main(void)
{
	int i;

	i = 1;
	if (procfs_test(&i))
		ksft_exit_fail();
	ksft_exit_pass();
}
