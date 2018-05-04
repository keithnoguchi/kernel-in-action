/* SPDX-License-Identifier: GPL-2.0 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int __init jit_init(void)
{
	pr_info("%s\n", __FUNCTION__);
	return 0;
}
module_init(jit_init);

static void __exit jit_exit(void)
{
	pr_info("%s\n", __FUNCTION__);
}
module_exit(jit_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("LDD's Just-In-Time module");
