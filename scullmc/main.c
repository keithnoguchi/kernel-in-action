/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>

#include "../ldd/ldd.h"

#define SCULLMC_DRIVER_VERSION		"1.0"
#define SCULLMC_DRIVER_NAME		"scullmc"
#define SCULLMC_DEFAULT_QUANTUM_SIZE	PAGE_SIZE

static const char *driver_version = SCULLMC_DRIVER_VERSION;
static const char *driver_name = SCULLMC_DRIVER_NAME;
static int quantum_size = SCULLMC_DEFAULT_QUANTUM_SIZE;
module_param(quantum_size, int, S_IRUGO);

static struct ldd_driver scullmc_driver = {
	.module = THIS_MODULE,
};

static struct scullmc_device {
	struct ldd_device	dev;
	struct cdev		cdev;
} scullmc_devices[] = {
	{ .dev.name = SCULLMC_DRIVER_NAME "0" },
	{ .dev.name = SCULLMC_DRIVER_NAME "1" },
	{ .dev.name = SCULLMC_DRIVER_NAME "2" },
	{ .dev.name = SCULLMC_DRIVER_NAME "3" },
	{ /* sentry */ },
};

/* kmem cache */
static struct kmem_cache *quantum_cache;

static ssize_t read(struct file *f, char __user *buf, size_t n, loff_t *pos)
{
	return 0;
}

static ssize_t write(struct file *f, const char __user *buf, size_t n, loff_t *pos)
{
	return 0;
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
	.owner		= THIS_MODULE,
	.read		= read,
	.write		= write,
	.open		= open,
	.release	= release,
};

static int __init init(void)
{
	struct scullmc_device *d, *del;
	int err;

	pr_info("%s\n", __FUNCTION__);

	quantum_cache = kmem_cache_create("scullmc_quantum", quantum_size,
					  0, SLAB_HWCACHE_ALIGN, NULL);
	if (!quantum_cache)
		return -ENOMEM;

	scullmc_driver.version = driver_version;
	scullmc_driver.driver.name = driver_name;
	err = register_ldd_driver(&scullmc_driver);
	if (err)
		goto destroy_cache;

	for (d = scullmc_devices; d->dev.name; d++) {
		cdev_init(&d->cdev, &fops);
		err = register_ldd_device(&d->dev);
		if (err)
			goto unregister;
	}
	return 0;
unregister:
	for (del = scullmc_devices; del != d; del++)
		unregister_ldd_device(&del->dev);
	unregister_ldd_driver(&scullmc_driver);
destroy_cache:
	if (quantum_cache)
		kmem_cache_destroy(quantum_cache);
	return err;
}
module_init(init);

static void __exit cleanup(void)
{
	struct scullmc_device *d;

	pr_info("%s\n", __FUNCTION__);

	for (d = scullmc_devices; d->dev.name; d++)
		unregister_ldd_device(&d->dev);
	unregister_ldd_driver(&scullmc_driver);
	if (quantum_cache)
		kmem_cache_destroy(quantum_cache);
}
module_exit(cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("scull kmem_cache_alloc() example");
