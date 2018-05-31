/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/fs.h>

#include "../ldd/ldd.h"

#define SCULLMC_DRIVER_VERSION		"1.3.2"
#define SCULLMC_DRIVER_NAME		"scullmc"
#define SCULLMC_DEFAULT_QUANTUM_SIZE	PAGE_SIZE

/* scullmc driver */
static struct scullmc_driver {
	dev_t			devt_base;
	struct kmem_cache	*qsetc;
	struct kmem_cache	*quantumc;
	struct ldd_driver	ldd;
} driver = {
	.ldd.module = THIS_MODULE,
};
#define to_driver(_drv)		container_of(to_ldd_driver(_drv), struct scullmc_driver, ldd)

/* scullmc devices */
static struct scullmc_device {
	struct qset		*qset;
	struct cdev		cdev;
	struct ldd_device	ldd;
} devices[] = {
	{ .ldd.name = SCULLMC_DRIVER_NAME "0" },
	{ .ldd.name = SCULLMC_DRIVER_NAME "1" },
	{ .ldd.name = SCULLMC_DRIVER_NAME "2" },
	{ .ldd.name = SCULLMC_DRIVER_NAME "3" },
	{ /* sentinel */ },
};

/* quantum set */
struct qset {
	struct qset	*next;
	void		*data[];
};

static const char *driver_version = SCULLMC_DRIVER_VERSION;
static const char *driver_name = SCULLMC_DRIVER_NAME;
static int quantum_size = SCULLMC_DEFAULT_QUANTUM_SIZE;
module_param(quantum_size, int, S_IRUGO);

static ssize_t read(struct file *f, char __user *buf, size_t n, loff_t *pos)
{
	struct scullmc_device *d = f->private_data;

	pr_info("%s(%s)\n", __FUNCTION__, ldd_dev_name(&d->ldd));

	return 0;
}

static ssize_t write(struct file *f, const char __user *buf, size_t n, loff_t *pos)
{
	struct scullmc_device *d = f->private_data;

	pr_info("%s(%s)\n", __FUNCTION__, ldd_dev_name(&d->ldd));

	return n;
}

static int open(struct inode *i, struct file *f)
{
	struct scullmc_device *d = container_of(i->i_cdev, struct scullmc_device, cdev);
	struct scullmc_driver *drv = to_driver(d->ldd.dev.driver);
	struct qset *qset;

	pr_info("%s(%s)\n", __FUNCTION__, ldd_dev_name(&d->ldd));

	qset = kmem_cache_alloc(drv->qsetc, GFP_KERNEL);
	if (!qset)
		return -ENOMEM;
	d->qset = qset;
	f->private_data = d;

	return 0;
}

static int release(struct inode *i, struct file *f)
{
	struct scullmc_device *d = f->private_data;
	struct scullmc_driver *drv = to_driver(d->ldd.dev.driver);

	pr_info("%s(%s)\n", __FUNCTION__, ldd_dev_name(&d->ldd));
	f->private_data = NULL;

	if (d->qset)
		kmem_cache_free(drv->qsetc, d->qset);
	d->qset = NULL;

	return 0;
}

static const struct file_operations fops = {
	.owner		= THIS_MODULE,
	.read		= read,
	.write		= write,
	.open		= open,
	.release	= release,
};

static int register_device(struct scullmc_device *d)
{
	int err;

	err = register_ldd_device(&d->ldd);
	if (err)
		return err;

	/* for cdev subsystem */
	cdev_init(&d->cdev, &fops);
	err = cdev_add(&d->cdev, d->ldd.dev.devt, 1);
	if (err)
		unregister_ldd_device(&d->ldd);

	return err;
}

static void unregister_device(struct scullmc_device *d)
{
	cdev_del(&d->cdev);
	unregister_ldd_device(&d->ldd);
}

static int __init init(void)
{
	struct scullmc_device *d, *del;
	int err;
	int i;

	pr_info("%s\n", __FUNCTION__);

	err = -ENOMEM;
	driver.qsetc = kmem_cache_create("scullmc_qset", sizeof(struct qset),
					 0, SLAB_HWCACHE_ALIGN, NULL);
	if (!driver.qsetc)
		goto destroy_caches;

	driver.quantumc = kmem_cache_create("scullmc_quantum", quantum_size,
					    0, SLAB_HWCACHE_ALIGN, NULL);
	if (!driver.quantumc)
		goto destroy_caches;

	err = alloc_chrdev_region(&driver.devt_base, 0, ARRAY_SIZE(devices),
				  SCULLMC_DRIVER_NAME);
	if (err)
		goto destroy_caches;

	driver.ldd.version = driver_version;
	driver.ldd.driver.name = driver_name;
	err = register_ldd_driver(&driver.ldd);
	if (err)
		goto unregister_chrdev;

	for (i = 0, d = devices; d->ldd.name; i++, d++) {
		/* for /dev/scullmcX file */
		d->ldd.dev.devt = MKDEV(MAJOR(driver.devt_base),
					MINOR(driver.devt_base)+i);
		err = register_device(d);
		if (err)
			goto unregister;
	}

	return 0;
unregister:
	for (del = devices; del != d; del++)
		unregister_device(del);
	unregister_ldd_driver(&driver.ldd);
unregister_chrdev:
	unregister_chrdev_region(driver.devt_base, ARRAY_SIZE(devices));
destroy_caches:
	if (driver.quantumc)
		kmem_cache_destroy(driver.quantumc);
	if (driver.qsetc)
		kmem_cache_destroy(driver.qsetc);
	return err;
}
module_init(init);

static void __exit cleanup(void)
{
	struct scullmc_device *d;

	pr_info("%s\n", __FUNCTION__);

	for (d = devices; d->ldd.name; d++)
		unregister_device(d);
	unregister_ldd_driver(&driver.ldd);
	unregister_chrdev_region(driver.devt_base, ARRAY_SIZE(devices));
	if (driver.quantumc)
		kmem_cache_destroy(driver.quantumc);
	if (driver.qsetc)
		kmem_cache_destroy(driver.qsetc);
}
module_exit(cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("scull kmem_cache_alloc() example");
