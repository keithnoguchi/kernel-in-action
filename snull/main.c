/* SPDX-License-Identifier: GPL-2.0 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int __init snull_init(void)
{
	pr_info("%s\n", __FUNCTION__);
	return 0;
}
module_init(snull_init);

static void __exit snull_exit(void)
{
	pr_info("%s\n", __FUNCTION__);
}
module_exit(snull_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("LDD's Simple Network device");
