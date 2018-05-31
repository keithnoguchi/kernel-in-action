/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/fs.h>

#include "../ldd/ldd.h"

#define SCULLMC_DRIVER_VERSION		"1.3"
#define SCULLMC_DRIVER_NAME		"scullmc"
#define SCULLMC_DEFAULT_QUANTUM_SIZE	PAGE_SIZE

static const char *driver_version = SCULLMC_DRIVER_VERSION;
static const char *driver_name = SCULLMC_DRIVER_NAME;
static int quantum_size = SCULLMC_DEFAULT_QUANTUM_SIZE;
module_param(quantum_size, int, S_IRUGO);

/* driver */
static struct scullmc_driver {
	dev_t			devt_base;
	struct kmem_cache	*quantumc;
	struct ldd_driver	drv;
} driver = {
	.drv.module = THIS_MODULE,
};

/* devices */
static struct scullmc_device {
	struct cdev		cdev;
	struct ldd_device	dev;
} scullmc_devices[] = {
	{ .dev.name = SCULLMC_DRIVER_NAME "0" },
	{ .dev.name = SCULLMC_DRIVER_NAME "1" },
	{ .dev.name = SCULLMC_DRIVER_NAME "2" },
	{ .dev.name = SCULLMC_DRIVER_NAME "3" },
	{ /* sentry */ },
};

static ssize_t read(struct file *f, char __user *buf, size_t n, loff_t *pos)
{
	struct scullmc_device *d = f->private_data;
	pr_info("%s(%s)\n", __FUNCTION__, ldd_dev_name(&d->dev));
	return 0;
}

static ssize_t write(struct file *f, const char __user *buf, size_t n, loff_t *pos)
{
	struct scullmc_device *d = f->private_data;
	pr_info("%s(%s)\n", __FUNCTION__, ldd_dev_name(&d->dev));
	return 0;
}

static int open(struct inode *i, struct file *f)
{
	struct scullmc_device *d = container_of(i->i_cdev, struct scullmc_device, cdev);
	pr_info("%s(%s)\n", __FUNCTION__, ldd_dev_name(&d->dev));
	f->private_data = d;
	return 0;
}

static int release(struct inode *i, struct file *f)
{
	struct scullmc_device *d = f->private_data;
	pr_info("%s(%s)\n", __FUNCTION__, ldd_dev_name(&d->dev));
	f->private_data = NULL;
	return 0;
}

static const struct file_operations fops = {
	.owner		= THIS_MODULE,
	.read		= read,
	.write		= write,
	.open		= open,
	.release	= release,
};

static int register_device(struct scullmc_device *d, dev_t devt)
{
	int err;

	/* add /dev/scullmc[0-4] */
	d->dev.dev.devt = devt;
	err = register_ldd_device(&d->dev);
	if (err)
		return err;

	/* cdev subsystem */
	cdev_init(&d->cdev, &fops);
	err = cdev_add(&d->cdev, devt, 1);
	if (err)
		unregister_ldd_device(&d->dev);

	return err;
}

static void unregister_device(struct scullmc_device *d)
{
	cdev_del(&d->cdev);
	unregister_ldd_device(&d->dev);
}

static int __init init(void)
{
	struct scullmc_device *d, *del;
	int err;
	int i;

	pr_info("%s\n", __FUNCTION__);

	driver.quantumc = kmem_cache_create("scullmc_quantum", quantum_size,
					    0, SLAB_HWCACHE_ALIGN, NULL);
	if (!driver.quantumc)
		return -ENOMEM;

	err = alloc_chrdev_region(&driver.devt_base, 0,
				  ARRAY_SIZE(scullmc_devices),
				  SCULLMC_DRIVER_NAME);
	if (err)
		goto destroy_cache;

	driver.drv.version = driver_version;
	driver.drv.driver.name = driver_name;
	err = register_ldd_driver(&driver.drv);
	if (err)
		goto unregister_chrdev;

	for (i = 0, d = scullmc_devices; d->dev.name; i++, d++) {
		dev_t devt = MKDEV(MAJOR(driver.devt_base),
				   MINOR(driver.devt_base)+i);
		err = register_device(d, devt);
		if (err)
			goto unregister;
	}

	return 0;
unregister:
	for (del = scullmc_devices; del != d; del++)
		unregister_device(del);
	unregister_ldd_driver(&driver.drv);
unregister_chrdev:
	unregister_chrdev_region(driver.devt_base,
				 ARRAY_SIZE(scullmc_devices));
destroy_cache:
	if (driver.quantumc)
		kmem_cache_destroy(driver.quantumc);
	return err;
}
module_init(init);

static void __exit cleanup(void)
{
	struct scullmc_device *d;

	pr_info("%s\n", __FUNCTION__);

	for (d = scullmc_devices; d->dev.name; d++)
		unregister_device(d);
	unregister_ldd_driver(&driver.drv);
	unregister_chrdev_region(driver.devt_base,
				 ARRAY_SIZE(scullmc_devices));
	if (driver.quantumc)
		kmem_cache_destroy(driver.quantumc);
}
module_exit(cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("scull kmem_cache_alloc() example");
