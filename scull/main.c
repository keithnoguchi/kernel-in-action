/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>

static int __init scull_init(void)
{
	pr_info("%s\n", __FUNCTION__);
	return 0;
}
module_init(scull_init);

static void __exit scull_exit(void)
{
	pr_info("%s\n", __FUNCTION__);
}
module_exit(scull_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("LDD scull character device driver");
