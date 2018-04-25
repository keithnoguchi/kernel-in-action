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

#define SCULL_DEV_PREFIX  "scull"
#define NR_SCULL_DEV      4

/* scull device driver. */
struct scull {
	struct device dev;
	struct cdev cdev;
};

/* Global variables.  Will try to clean those up. */
static dev_t dev_number_base;
const char *scull_dev_name = SCULL_DEV_PREFIX;
static const int nr_dev = NR_SCULL_DEV;
static struct scull sculls[NR_SCULL_DEV];

/* File operations. */
static struct file_operations fops = {
	.owner = THIS_MODULE,
};

static int __init scull_init(void)
{
	struct scull *s;
	char buf[16];
	int i, j;
	int err;

	/* allocate char device number region */
	err = alloc_chrdev_region(&dev_number_base, 0, nr_dev, scull_dev_name);
	if (err)
		return err;

	/* create scull devices */
	for (s = sculls, i = 0; i < nr_dev; s++, i++) {
		device_initialize(&s->dev);
		s->dev.devt = MKDEV(MAJOR(dev_number_base), i);
		sprintf(buf, "%s%d", scull_dev_name, i);
		s->dev.init_name = buf;
		cdev_init(&s->cdev, &fops);
		s->cdev.owner = THIS_MODULE;

		/* add the device into the character device subsystem */
		err = cdev_device_add(&s->cdev, &s->dev);
		if (err)
			goto unregister;

		pr_info("%s[%d:%d]: added\n", dev_name(&s->dev),
			MAJOR(s->dev.devt), MINOR(s->dev.devt));
	}

	return 0;
unregister:
	/* only delete the added devices */
	for (s = sculls, j = 0; j < i; s++, j++) {
		cdev_device_del(&s->cdev, &s->dev);
		pr_info("%s[%d:%d]: deleted\n", dev_name(&s->dev),
			MAJOR(s->dev.devt), MINOR(s->dev.devt));
	}
	unregister_chrdev_region(dev_number_base, nr_dev);
	return err;
}
module_init(scull_init);

static void __exit scull_exit(void)
{
	struct scull *s;
	int i;

	for (s = sculls, i = 0; i < nr_dev; s++, i++) {
		cdev_device_del(&s->cdev, &s->dev);
		pr_info("%s[%d:%d]: deleted\n", dev_name(&s->dev),
			MAJOR(s->dev.devt), MINOR(s->dev.devt));
	}
	unregister_chrdev_region(dev_number_base, nr_dev);
}
module_exit(scull_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("LDD scull character device driver");
