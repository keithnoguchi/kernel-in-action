/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/mutex.h>

#include "../ldd/ldd.h"

#define SCULLCM_DRIVER_VERSION			"1.3.3"
#define SCULLCM_DRIVER_NAME			"scullcm"
#define SCULLCM_DEFAULT_QUANTUM_VECTOR_NR	8
#define SCULLCM_DEFAULT_QUANTUM_SIZE		PAGE_SIZE

/* scullcm driver */
static struct scullcm_driver {
	dev_t			devt_base;
	int			qvec_nr;
	size_t			qsize;
	struct kmem_cache	*qsetc;
	struct kmem_cache	*qvecc;
	struct kmem_cache	*quantumc;
	struct ldd_driver	ldd;
} driver = {
	.ldd.module = THIS_MODULE,
};
#define to_scullcm_driver(_drv)	container_of(to_ldd_driver(_drv), struct scullcm_driver, ldd)

/* scullcm devices */
static struct scullcm_device {
	struct mutex		lock;
	struct qset		*qhead;
	struct cdev		cdev;
	struct ldd_device	ldd;
} devices[] = {
	{ .ldd.name = SCULLCM_DRIVER_NAME "0" },
	{ .ldd.name = SCULLCM_DRIVER_NAME "1" },
	{ .ldd.name = SCULLCM_DRIVER_NAME "2" },
	{ .ldd.name = SCULLCM_DRIVER_NAME "3" },
	{ /* sentinel */ },
};

/* quantum set */
struct qset {
	struct qset	*next;
	void		**data;
};

static const char *driver_version = SCULLCM_DRIVER_VERSION;
static const char *driver_name = SCULLCM_DRIVER_NAME;
static int quantum_vector_number = SCULLCM_DEFAULT_QUANTUM_VECTOR_NR;
static int quantum_size = SCULLCM_DEFAULT_QUANTUM_SIZE;
module_param(quantum_vector_number, int, S_IRUGO);
module_param(quantum_size, int, S_IRUGO);

static struct qset *alloc_qset(struct scullcm_driver *drv)
{
	struct qset *q;

	q = kmem_cache_alloc(drv->qsetc, GFP_KERNEL);
	if (!q)
		goto err;
	q->data = kmem_cache_alloc(drv->qvecc, GFP_KERNEL);
	if (!q->data)
		goto err;
	q->next = NULL;
	return q;
err:
	if (q)
		kmem_cache_free(drv->qsetc, q);
	return ERR_PTR(-ENOMEM);
}

static void free_qset(struct scullcm_driver *drv, struct qset *q)
{
	if (q->data)
		kmem_cache_free(drv->qvecc, q->data);
	kmem_cache_free(drv->qsetc, q);
}

static void trim_qset(struct scullcm_driver *drv, struct qset **head)
{
	struct qset *q;

	while ((q = *head)) {
		*head = q->next;
		free_qset(drv, q);
	}
}

static ssize_t read(struct file *f, char __user *buf, size_t n, loff_t *pos)
{
	struct scullcm_device *d = f->private_data;

	pr_info("%s(%s)\n", __FUNCTION__, ldd_dev_name(&d->ldd));

	return 0;
}

static ssize_t write(struct file *f, const char __user *buf, size_t n, loff_t *pos)
{
	struct scullcm_device *d = f->private_data;

	pr_info("%s(%s)\n", __FUNCTION__, ldd_dev_name(&d->ldd));

	return n;
}

static int open(struct inode *i, struct file *f)
{
	struct scullcm_device *d = container_of(i->i_cdev, struct scullcm_device, cdev);
	struct scullcm_driver *drv = to_scullcm_driver(d->ldd.dev.driver);
	struct qset *q;
	int err = 0;

	pr_info("%s(%s)\n", __FUNCTION__, ldd_dev_name(&d->ldd));

	f->private_data = d;
	if (mutex_lock_interruptible(&d->lock))
		return -ERESTARTSYS;

	if (d->qhead)
		goto out;

	/* start with one quantum set */
	q = alloc_qset(drv);
	if (IS_ERR(q)) {
		err = PTR_ERR(q);
		goto out;
	}
	d->qhead = q;
out:
	mutex_unlock(&d->lock);
	return err;
}

static int release(struct inode *i, struct file *f)
{
	struct scullcm_device *d = f->private_data;
	struct scullcm_driver *drv = to_scullcm_driver(d->ldd.dev.driver);

	pr_info("%s(%s)\n", __FUNCTION__, ldd_dev_name(&d->ldd));

	if (mutex_lock_interruptible(&d->lock))
		return -ERESTARTSYS;
	trim_qset(drv, &d->qhead);
	mutex_unlock(&d->lock);
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

static int register_device(struct scullcm_device *d)
{
	int err;

	err = register_ldd_device(&d->ldd);
	if (err)
		return err;

	/* for cdev subsystem */
	cdev_init(&d->cdev, &fops);
	mutex_init(&d->lock);
	err = cdev_add(&d->cdev, d->ldd.dev.devt, 1);
	if (err)
		unregister_ldd_device(&d->ldd);

	return err;
}

static void unregister_device(struct scullcm_device *d)
{
	cdev_del(&d->cdev);
	unregister_ldd_device(&d->ldd);
}

static int __init init(void)
{
	struct scullcm_device *d, *del;
	int err = -ENOMEM;
	int i;

	pr_info("%s\n", __FUNCTION__);


	/* quantum set cache */
	driver.qsetc = kmem_cache_create("scullcm_qset", sizeof(struct qset),
					 0, SLAB_HWCACHE_ALIGN, NULL);
	if (!driver.qsetc)
		goto destroy_caches;

	/* quantum vector cache */
	driver.qvec_nr = quantum_vector_number;
	driver.qvecc = kmem_cache_create("scullcm_quantum_vector",
					 sizeof(void *)*driver.qvec_nr,
					 0, SLAB_HWCACHE_ALIGN, NULL);
	if (!driver.qvecc)
		goto destroy_caches;

	/* quantum cache */
	driver.qsize = quantum_size;
	driver.quantumc = kmem_cache_create("scullcm_quantum", driver.qsize,
					    0, SLAB_HWCACHE_ALIGN, NULL);
	if (!driver.quantumc)
		goto destroy_caches;

	err = alloc_chrdev_region(&driver.devt_base, 0, ARRAY_SIZE(devices),
				  SCULLCM_DRIVER_NAME);
	if (err)
		goto destroy_caches;

	driver.ldd.version = driver_version;
	driver.ldd.driver.name = driver_name;
	err = register_ldd_driver(&driver.ldd);
	if (err)
		goto unregister_chrdev;

	for (i = 0, d = devices; d->ldd.name; i++, d++) {
		/* for /dev/scullcmX file */
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
	if (driver.qvecc)
		kmem_cache_destroy(driver.qvecc);
	if (driver.qsetc)
		kmem_cache_destroy(driver.qsetc);
	return err;
}
module_init(init);

static void __exit cleanup(void)
{
	struct scullcm_device *d;

	pr_info("%s\n", __FUNCTION__);

	for (d = devices; d->ldd.name; d++)
		unregister_device(d);
	unregister_ldd_driver(&driver.ldd);
	unregister_chrdev_region(driver.devt_base, ARRAY_SIZE(devices));
	if (driver.quantumc)
		kmem_cache_destroy(driver.quantumc);
	if (driver.qvecc)
		kmem_cache_destroy(driver.qvecc);
	if (driver.qsetc)
		kmem_cache_destroy(driver.qsetc);
}
module_exit(cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("scull kmem_cache_alloc() example");
