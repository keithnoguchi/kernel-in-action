/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>

static dev_t dev_number_base;
const char *dev_name = "scull";
static int nr_dev = 1;

static int __init scull_init(void)
{
	int err;

	pr_info("%s\n", __FUNCTION__);
	err = alloc_chrdev_region(&dev_number_base, 0, nr_dev, dev_name);
	if (err)
		return err;
	pr_info("MAJOR=%d, MINOR=%d\n", MAJOR(dev_number_base),
		MINOR(dev_number_base));

	return 0;
}
module_init(scull_init);

static void __exit scull_exit(void)
{
	pr_info("%s\n", __FUNCTION__);
	unregister_chrdev_region(dev_number_base, nr_dev);
}
module_exit(scull_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("LDD scull character device driver");
