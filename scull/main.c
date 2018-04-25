/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>

/* scull device driver. */
struct scull {
	struct cdev cdev;
};

/* Global variables.  Will try to clean those up. */
static dev_t dev_number_base;
static struct class *cls = NULL;
const char *scull_dev_name = "scull";
static const int nr_dev = 1;
static struct scull sculls[1];

/* File operations. */
static struct file_operations fops = {
	.owner = THIS_MODULE,
};

static int __init scull_init(void)
{
	struct scull *dev;
	int idx = 0;
	int err;

	pr_info("%s\n", __FUNCTION__);

	/* allocate char device number */
	err = alloc_chrdev_region(&dev_number_base, idx, nr_dev, scull_dev_name);
	if (err)
		return err;
	pr_info("MAJOR=%d, MINOR=%d\n", MAJOR(dev_number_base),
		MINOR(dev_number_base));

	cls = class_create(THIS_MODULE, scull_dev_name);
	if (IS_ERR(cls))
		return PTR_ERR(cls);

	/* initialize and add it to the char subsystem */
	dev = &sculls[idx];
	cdev_init(&dev->cdev, &fops);
	dev->cdev.owner = THIS_MODULE;
	err = cdev_add(&dev->cdev, dev_number_base, nr_dev);
	if (err)
		goto unregister;

	return 0;
unregister:
	unregister_chrdev_region(dev_number_base, nr_dev);
	return err;
}
module_init(scull_init);

static void __exit scull_exit(void)
{
	struct scull *dev;
	int idx = 0;

	pr_info("%s\n", __FUNCTION__);
	dev = &sculls[idx];
	cdev_del(&dev->cdev);
	if (!IS_ERR_OR_NULL(cls))
		class_destroy(cls);
	unregister_chrdev_region(dev_number_base, nr_dev);
}
module_exit(scull_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("LDD scull character device driver");
