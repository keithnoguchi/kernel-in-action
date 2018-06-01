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
#include <linux/uaccess.h>

#include "../ldd/ldd.h"

#define SCULLCM_DRIVER_VERSION			"1.3.4"
#define SCULLCM_DRIVER_NAME			"scullcm"
#define SCULLCM_DEVICE_PREFIX			SCULLCM_DRIVER_NAME
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
} scullcm = {
	.ldd.module = THIS_MODULE,
};
#define to_scullcm_driver(_drv)	container_of(to_ldd_driver(_drv), struct scullcm_driver, ldd)

/* scullcm devices */
static struct scullcm_device {
	struct mutex		lock;
	size_t			size;
	struct qset		*qhead;
	struct cdev		cdev;
	struct ldd_device	ldd;
	struct device_attribute	size_attr;
} devices[] = {
	{ .ldd.name = SCULLCM_DEVICE_PREFIX "0" },
	{ .ldd.name = SCULLCM_DEVICE_PREFIX "1" },
	{ .ldd.name = SCULLCM_DEVICE_PREFIX "2" },
	{ .ldd.name = SCULLCM_DEVICE_PREFIX "3" },
	{ /* sentinel */ },
};
#define to_scullcm_device(_d)	container_of(to_ldd_device(_d), struct scullcm_device, ldd)

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
	struct qset *s;

	s = kmem_cache_alloc(drv->qsetc, GFP_KERNEL);
	if (!s)
		goto err;
	s->data = kmem_cache_alloc(drv->qvecc, GFP_KERNEL);
	if (!s->data)
		goto err;
	memset(s->data, 0, sizeof(void *)*drv->qvec_nr);
	s->next = NULL;
	return s;
err:
	if (s)
		kmem_cache_free(drv->qsetc, s);
	return ERR_PTR(-ENOMEM);
}

static void free_qset(struct scullcm_driver *drv, struct qset *s)
{
	if (s->data) {
		int i;
		for (i = 0; i < drv->qvec_nr; i++)
			if (s->data[i])
				kmem_cache_free(drv->quantumc, s->data[i]);
		kmem_cache_free(drv->qvecc, s->data);
	}
	kmem_cache_free(drv->qsetc, s);
}

static void trim_qset(struct scullcm_device *d)
{
	struct scullcm_driver *drv = to_scullcm_driver(d->ldd.dev.driver);
	struct qset *s;

	while ((s = d->qhead)) {
		d->qhead = s->next;
		free_qset(drv, s);
	}
	d->size = 0;
}

static void *get_quantum(struct scullcm_driver *drv, struct qset *s, int s_pos)
{
	if (!s->data[s_pos])
		s->data[s_pos] = kmem_cache_alloc(drv->quantumc, GFP_KERNEL);
	return s->data[s_pos];
}

static ssize_t read(struct file *f, char __user *buf, size_t n, loff_t *pos)
{
	struct scullcm_device *d = f->private_data;
	struct scullcm_driver *drv = to_scullcm_driver(d->ldd.dev.driver);
	int s_pos, q_pos;
	void *q;
	int ret;

	pr_info("%s(%s)\n", __FUNCTION__, ldd_dev_name(&d->ldd));

	/* no more data to read */
	if (*pos >= d->size)
		return 0;

	/* find the quantum */
	s_pos = *pos/drv->qsize;
	q_pos = *pos%drv->qsize;

	/* only single quantum read */
	if (q_pos+n > drv->qsize)
		n = drv->qsize-q_pos;

	if (mutex_lock_interruptible(&d->lock))
		return -ERESTARTSYS;

	/* no more data */
	if (*pos == d->size) {
		ret = 0;
		goto out;
	}
	/* only read the remaining data */
	if (*pos + n >= d->size)
		n = d->size-*pos;

	/* find the quantum */
	ret = -EINVAL;
	q = d->qhead->data[s_pos];
	if (!q)
		goto out;

	/* copy to the user */
	ret = -EINVAL;
	if (copy_to_user(buf, q+q_pos, n))
		goto out;
	*pos += n;
	ret = n;
out:
	mutex_unlock(&d->lock);
	return ret;
}

static ssize_t write(struct file *f, const char __user *buf, size_t n, loff_t *pos)
{
	struct scullcm_device *d = f->private_data;
	struct scullcm_driver *drv = to_scullcm_driver(d->ldd.dev.driver);
	int s_pos, q_pos;
	void *q;
	int ret;

	pr_info("%s(%s)\n", __FUNCTION__, ldd_dev_name(&d->ldd));

	/* find the quantum position */
	s_pos = *pos/drv->qsize;
	q_pos = *pos%drv->qsize;

	/* no multi qsets support */
	if (s_pos >= drv->qvec_nr)
		return -EINVAL;

	/* no multi quantum write */
	if (q_pos+n > drv->qsize)
		n = drv->qsize-q_pos;

	if (mutex_lock_interruptible(&d->lock))
		return -ERESTARTSYS;

	ret = -ENOMEM;
	q = get_quantum(drv, d->qhead, s_pos);
	if (!q)
		goto out;

	ret = -EINVAL;
	if (copy_from_user(q+q_pos, buf, n))
		goto out;
	*pos += n;
	ret = n;
	if (*pos > d->size)
		d->size = *pos;
out:
	mutex_unlock(&d->lock);
	return ret;
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

	/* trim the qset when it opened write only with trunk option */
	if ((f->f_flags&O_ACCMODE) == O_WRONLY && f->f_flags&O_TRUNC)
		trim_qset(d);
	else if (d->qhead)
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

	pr_info("%s(%s)\n", __FUNCTION__, ldd_dev_name(&d->ldd));
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

static ssize_t show_device_size(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct scullcm_device *s = to_scullcm_device(dev);
	return snprintf(buf, PAGE_SIZE, "%ld\n", s->size);
}

static int register_device_attr(struct scullcm_device *d)
{
	const struct device_attribute size_attr = {
		.attr.name	= "size",
		.attr.mode	= S_IRUGO,
		.show		= show_device_size,
	};
	d->size_attr = size_attr;
	return device_create_file(&d->ldd.dev, &d->size_attr);
}

static void unregister_device_attr(struct scullcm_device *d)
{
	device_remove_file(&d->ldd.dev, &d->size_attr);
}

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
		goto unregister;
	err = register_device_attr(d);
	if (err)
		goto unregister;
	return 0;
unregister:
	unregister_ldd_device(&d->ldd);
	return err;
}

static void unregister_device(struct scullcm_device *d)
{
	trim_qset(d);
	cdev_del(&d->cdev);
	unregister_device_attr(d);
	unregister_ldd_device(&d->ldd);
}

static ssize_t show_driver_qvec_nr(struct device_driver *drv, char *buf)
{
	struct scullcm_driver *s = to_scullcm_driver(drv);
	return snprintf(buf, PAGE_SIZE, "%d\n", s->qvec_nr);
}

static ssize_t show_driver_qsize(struct device_driver *drv, char *buf)
{
	struct scullcm_driver *s = to_scullcm_driver(drv);
	return snprintf(buf, PAGE_SIZE, "%ld\n", s->qsize);
}

static const struct driver_attribute qvec_nr_attr = {
	.attr.name	= "quantum_vector_number",
	.attr.mode	= S_IRUGO,
	.show		= show_driver_qvec_nr,
};

static const struct driver_attribute qsize_attr = {
	.attr.name	= "qsize",
	.attr.mode	= S_IRUGO,
	.show		= show_driver_qsize,
};

static int register_driver_attr(struct scullcm_driver *drv)
{
	int err;

	err = driver_create_file(&drv->ldd.driver, &qvec_nr_attr);
	if (err)
		return err;
	err = driver_create_file(&drv->ldd.driver, &qsize_attr);
	if (err)
		driver_remove_file(&drv->ldd.driver, &qvec_nr_attr);
	return err;
}

static void unregister_driver_attr(struct scullcm_driver *drv)
{
	driver_remove_file(&drv->ldd.driver, &qvec_nr_attr);
	driver_remove_file(&drv->ldd.driver, &qsize_attr);
}

static int register_driver(struct scullcm_driver *drv)
{
	int err = -ENOMEM;

	/* quantum set cache */
	drv->qsetc = kmem_cache_create("scullcm_qset", sizeof(struct qset),
				       0, SLAB_HWCACHE_ALIGN, NULL);
	if (!drv->qsetc)
		goto destroy_caches;

	/* quantum vector cache */
	drv->qvec_nr = quantum_vector_number;
	drv->qvecc = kmem_cache_create("scullcm_quantum_vector",
				       sizeof(void *)*drv->qvec_nr,
				       0, SLAB_HWCACHE_ALIGN, NULL);
	if (!drv->qvecc)
		goto destroy_caches;

	/* quantum cache */
	drv->qsize = quantum_size;
	drv->quantumc = kmem_cache_create("scullcm_quantum", drv->qsize,
					  0, SLAB_HWCACHE_ALIGN, NULL);
	if (!drv->quantumc)
		goto destroy_caches;

	err = alloc_chrdev_region(&drv->devt_base, 0, ARRAY_SIZE(devices),
				  SCULLCM_DRIVER_NAME);
	if (err)
		goto destroy_caches;

	drv->ldd.version = driver_version;
	drv->ldd.driver.name = driver_name;
	err = register_ldd_driver(&drv->ldd);
	if (err)
		goto unregister_chrdev_region;

	err = register_driver_attr(drv);
	if (err)
		goto unregister_driver;

	return err;
unregister_driver:
	unregister_ldd_driver(&drv->ldd);
unregister_chrdev_region:
	unregister_chrdev_region(scullcm.devt_base, ARRAY_SIZE(devices));
destroy_caches:
	if (drv->quantumc)
		kmem_cache_destroy(drv->quantumc);
	if (drv->qvecc)
		kmem_cache_destroy(drv->qvecc);
	if (drv->qsetc)
		kmem_cache_destroy(drv->qsetc);
	return err;
}

static void unregister_driver(struct scullcm_driver *drv)
{
	unregister_driver_attr(drv);
	unregister_ldd_driver(&drv->ldd);
	unregister_chrdev_region(drv->devt_base, ARRAY_SIZE(devices));
	if (drv->quantumc)
		kmem_cache_destroy(drv->quantumc);
	if (drv->qvecc)
		kmem_cache_destroy(drv->qvecc);
	if (drv->qsetc)
		kmem_cache_destroy(drv->qsetc);
}

static int __init init(void)
{
	struct scullcm_device *d, *del;
	int err;
	int i;

	pr_info("%s\n", __FUNCTION__);

	err = register_driver(&scullcm);
	if (err)
		return err;

	for (i = 0, d = devices; d->ldd.name; i++, d++) {
		/* for /dev/scullcmX file */
		d->ldd.dev.devt = MKDEV(MAJOR(scullcm.devt_base),
					MINOR(scullcm.devt_base)+i);
		err = register_device(d);
		if (err)
			goto unregister;
	}
	return err;
unregister:
	for (del = devices; del != d; del++)
		unregister_device(del);
	unregister_driver(&scullcm);
	return err;
}
module_init(init);

static void __exit cleanup(void)
{
	struct scullcm_device *d;

	pr_info("%s\n", __FUNCTION__);

	for (d = devices; d->ldd.name; d++)
		unregister_device(d);
	unregister_driver(&scullcm);
}
module_exit(cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("scull kmem_cache_alloc() example");
