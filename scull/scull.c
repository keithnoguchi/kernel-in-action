/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int scull_init(void)
{
	printk("scull_init()\n");
	return 0;
}
module_init(scull_init);

static void scull_exit(void)
{
	printk("scull_exit()\n");
}
module_exit(scull_exit);

MODULE_LICENSE("GPL-2.0");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
