/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>

#include "../ldd/ldd.h"

#define SCULLPM_DRIVER_NAME	"scullpm"
#define SCULLPM_DRIVER_VERSION	"1.0.0"
#define SCULLPM_DEVICE_PREFIX	SCULLPM_DRIVER_NAME

/* driver */
static struct scullpm_driver {
	dev_t			devt;
	struct ldd_driver	ldd;
} scullpm = {
	.ldd.module		= THIS_MODULE,
	.ldd.version		= SCULLPM_DRIVER_VERSION,
	.ldd.driver.name	= SCULLPM_DRIVER_NAME,
};

/* devices */
static struct scullpm_device {
	struct cdev		cdev;
	struct ldd_device	ldd;
} devices[] = {
	{ .ldd.name	= SCULLPM_DEVICE_PREFIX "0" },
	{ .ldd.name	= SCULLPM_DEVICE_PREFIX "1" },
	{ /* sentinel */ },
};
#define to_scullpm_device(_dev)	container_of(to_ldd_device(_dev), struct scullpm_device, ldd)

static ssize_t read(struct file *f, char __user *buf, size_t n, loff_t *pos)
{
	return -ENOTTY;
}

static ssize_t write(struct file *f, const char __user *buf, size_t n, loff_t *pos)
{
	return -ENOTTY;
}

static int open(struct inode *i, struct file *f)
{
	return 0;
}

static int release(struct inode *i, struct file *f)
{
	return 0;
}

static const struct file_operations fops = {
	.read		= read,
	.write		= write,
	.open		= open,
	.release	= release,
};

static ssize_t show_major_number(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", MAJOR(dev->devt));
}

static const struct device_attribute major_attr = {
	.attr.name	= "major_number",
	.attr.mode	= S_IRUGO,
	.show		= show_major_number,
};

static ssize_t show_minor_number(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", MINOR(dev->devt));
}

static const struct device_attribute minor_attr = {
	.attr.name	= "minor_number",
	.attr.mode	= S_IRUGO,
	.show		= show_minor_number,
};

static int register_device(struct scullpm_device *d, dev_t devt)
{
	int err;

	/* for /dev/scullpmX */
	d->ldd.dev.devt = devt;
	err = register_ldd_device(&d->ldd);
	if (err)
		return err;

	/* device attributes */
	err = device_create_file(&d->ldd.dev, &major_attr);
	if (err)
		goto unregister;
	err = device_create_file(&d->ldd.dev, &minor_attr);
	if (err) {
		device_remove_file(&d->ldd.dev, &major_attr);
		goto unregister;
	}

	/* register in the char dev subsystem */
	cdev_init(&d->cdev, &fops);
	err = cdev_add(&d->cdev, devt, 1);
	if (err)
		goto remove_attribute;

	return 0;
remove_attribute:
	device_remove_file(&d->ldd.dev, &major_attr);
	device_remove_file(&d->ldd.dev, &minor_attr);
unregister:
	unregister_ldd_device(&d->ldd);
	return err;
}

static void unregister_device(struct scullpm_device *d)
{
	cdev_del(&d->cdev);
	device_remove_file(&d->ldd.dev, &major_attr);
	device_remove_file(&d->ldd.dev, &minor_attr);
	unregister_ldd_device(&d->ldd);
}

static int register_driver(struct scullpm_driver *drv)
{
	size_t nr = ARRAY_SIZE(devices);
	int err;

	err = register_ldd_driver(&drv->ldd);
	if (err)
		return err;

	err = alloc_chrdev_region(&drv->devt, 0, nr, drv->ldd.driver.name);
	if (err)
		goto unregister;

	return err;
unregister:
	unregister_ldd_driver(&drv->ldd);
	return err;
}

static void unregister_driver(struct scullpm_driver *drv)
{
	unregister_ldd_driver(&drv->ldd);
	unregister_chrdev_region(drv->devt, ARRAY_SIZE(devices));
}

static int __init init(void)
{
	struct scullpm_driver *drv = &scullpm;
	struct scullpm_device *d, *del;
	int err;
	int i;

	pr_info("%s\n", __FUNCTION__);

	err = register_driver(drv);
	if (err)
		return err;

	for (i = 0, d = devices; d->ldd.name; i++, d++) {
		err = register_device(d, MKDEV(MAJOR(drv->devt),
					       MINOR(drv->devt)+i));
		if (err)
			goto unregister;
	}
	return 0;
unregister:
	for (del = devices; del != d; del++)
		unregister_device(del);
	unregister_driver(drv);
	return err;
}
module_init(init);

static void __exit cleanup(void)
{
	struct scullpm_driver *drv = &scullpm;
	struct scullpm_device *d;

	pr_info("%s\n", __FUNCTION__);
	for (d = devices; d->ldd.name; d++)
		unregister_device(d);
	unregister_driver(drv);
}
module_exit(cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("scullpm: Page based scull");
