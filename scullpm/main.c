/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int __init init(void)
{
	pr_info("%s\n", __FUNCTION__);
	return 0;
}
module_init(init);

static void __exit cleanup(void)
{
	pr_info("%s\n", __FUNCTION__);
}
module_exit(cleanup);

MODULE_LICENSE("GPL-2.0");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("scullpm: Page based scull");
