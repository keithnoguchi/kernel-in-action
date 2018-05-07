/* SPDX-License-Identifier: GPL-2.0 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int __init lspci_init(void)
{
	pr_info("%s\n", __FUNCTION__);
	return 0;
}
module_init(lspci_init);

static void __exit lspci_exit(void)
{
	pr_info("%s\n", __FUNCTION__);
}
module_exit(lspci_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("Simple module to list the PCI devices");




