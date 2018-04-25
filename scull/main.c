/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>

/* scull device driver. */
struct scull {
	struct device dev;
	struct cdev cdev;
};

/* Global variables.  Will try to clean those up. */
static dev_t dev_number_base;
const char *scull_dev_name = "scull";
static const int nr_dev = 1;
static struct scull sculls[1];

/* File operations. */
static struct file_operations fops = {
	.owner = THIS_MODULE,
};

static int __init scull_init(void)
{
	struct scull *s;
	char buf[16];
	int idx = 0;
	int err;

	pr_info("%s\n", __FUNCTION__);

	/* allocate char device number region */
	err = alloc_chrdev_region(&dev_number_base, idx, nr_dev, scull_dev_name);
	if (err)
		return err;
	pr_info("MAJOR=%d, MINOR=%d\n", MAJOR(dev_number_base),
		MINOR(dev_number_base));

	/* initialize the scull driver. */
	s = &sculls[idx];
	sprintf(buf, "%s%d", scull_dev_name, idx);
	device_initialize(&s->dev);
	s->dev.init_name = buf;
	s->dev.devt = MKDEV(MAJOR(dev_number_base), idx);
	cdev_init(&s->cdev, &fops);
	s->cdev.owner = THIS_MODULE;

	/* register the device into the character device subsystem */
	err = cdev_device_add(&s->cdev, &s->dev);
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
	struct scull *s;
	int i;

	pr_info("%s\n", __FUNCTION__);
	for (s = sculls, i = 0; i < nr_dev; s++, i++)
		cdev_device_del(&s->cdev, &s->dev);
	unregister_chrdev_region(dev_number_base, nr_dev);
}
module_exit(scull_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("LDD scull character device driver");
