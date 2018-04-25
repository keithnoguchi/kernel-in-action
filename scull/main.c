/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>

#define NR_SCULL_DEV		4
#define SCULL_DEV_PREFIX	"scull"
#define SCULL_DEV_NAME_LEN	(strlen(SCULL_DEV_PREFIX) + 2)

/* scull device descriptor. */
static struct scull {
	struct device		dev;
	struct cdev		cdev;
} sculls[NR_SCULL_DEV];

/* File operations. */
static int scull_open(struct inode *i, struct file *f)
{
	pr_info("%s\n", __FUNCTION__);
	return 0;
}

static ssize_t scull_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
	pr_info("%s\n", __FUNCTION__);
	return 0;
}

static ssize_t scull_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
	pr_info("%s\n", __FUNCTION__);
	return len;
}

static int scull_release(struct inode *i, struct file *f)
{
	pr_info("%s\n", __FUNCTION__);
	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = scull_open,
	.read = scull_read,
	.write = scull_write,
	.release = scull_release,
};

static int __init scull_init(void)
{
	const int nr_dev = ARRAY_SIZE(sculls);
	char buf[SCULL_DEV_NAME_LEN];
	dev_t dev_base;
	struct scull *s;
	int i, j;
	int err;

	/* allocate the device number region */
	err = alloc_chrdev_region(&dev_base, 0, nr_dev, SCULL_DEV_PREFIX);
	if (err)
		return err;

	/* create scull devices */
	for (s = sculls, i = 0; i < nr_dev; s++, i++) {
		device_initialize(&s->dev);
		s->dev.devt = MKDEV(MAJOR(dev_base), MINOR(dev_base) + i);
		sprintf(buf, SCULL_DEV_PREFIX "%d", i);
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
	unregister_chrdev_region(dev_base, nr_dev);
	return err;
}
module_init(scull_init);

static void __exit scull_exit(void)
{
	const int nr_dev = ARRAY_SIZE(sculls);
	dev_t dev_base = sculls[0].dev.devt;
	struct scull *s;
	int i;

	for (s = sculls, i = 0; i < nr_dev; s++, i++) {
		cdev_device_del(&s->cdev, &s->dev);
		pr_info("%s[%d:%d]: deleted\n", dev_name(&s->dev),
			MAJOR(s->dev.devt), MINOR(s->dev.devt));
	}
	unregister_chrdev_region(dev_base, nr_dev);
}
module_exit(scull_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("LDD scull character device driver");
