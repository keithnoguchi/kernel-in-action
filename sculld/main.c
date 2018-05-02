/* SPDX-License-Identifier: GPL-2.0 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int __init sculld_init(void)
{
	pr_info("%s\n", __FUNCTION__);
	return 0;
}
module_init(sculld_init);

static void __exit sculld_exit(void)
{
	pr_info("%s\n", __FUNCTION__);
}
module_exit(sculld_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("LDD's scull driver under ldd virtual bus");
