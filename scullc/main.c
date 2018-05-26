/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

static int __init scullc_init_module(void)
{
	pr_info("%s\n", __FUNCTION__);
	return 0;
}
module_init(scullc_init_module);

static void __exit scullc_exit_module(void)
{
	pr_info("%s\n", __FUNCTION__);
}
module_exit(scullc_exit_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("kmem_cache_alloc() example");


