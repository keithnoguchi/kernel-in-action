/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fs.h>

/* scull globals */
static struct scull {
	dev_t		dev_base;
	int		dev_count;
	const char	*dev_name;
} scull = {
	.dev_name	= "scull",
	.dev_count	= 1,
};

static int scull_init(void)
{
	int err;

	err = alloc_chrdev_region(&scull.dev_base, 0, scull.dev_count, scull.dev_name);
	if (err < 0)
		goto out;
out:
	printk("scull_init(%d:%d)\n", MAJOR(scull.dev_base), MINOR(scull.dev_base));
	return err;
}
module_init(scull_init);

static void scull_exit(void)
{
	printk("scull_exit(%d:%d)\n", MAJOR(scull.dev_base), MINOR(scull.dev_base));
	unregister_chrdev_region(scull.dev_base, scull.dev_count);
}
module_exit(scull_exit);

MODULE_LICENSE("GPL-2.0");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
