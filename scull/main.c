/* SPDX-License-Identifier: GPL-2.0 */

#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <asm/page.h>

#define NR_SCULL_DEV		4
#define NR_SCULL_QSET           1000
#define SCULL_QUANTUM_SIZE      PAGE_SIZE
#define SCULL_DEV_PREFIX	"scull"
#define SCULL_DEV_NAME_LEN	(strlen(SCULL_DEV_PREFIX) + 2)

/* scull device descriptor. */
static struct scull {
	struct semaphore	lock;
	struct device		dev;
	struct cdev		cdev;
	struct scull_qset	*data;
	int			qset;
	int			quantum;
} sculls[NR_SCULL_DEV];

/* quantum set. */
struct scull_qset {
	void			**data;
	struct scull_qset	*next;
};

/* quantum set size. */
static int scull_qset = NR_SCULL_QSET;
static int scull_quantum = SCULL_QUANTUM_SIZE;

/* allocate the quantum set. */
static struct scull_qset *alloc_qset(struct scull *s)
{
	struct scull_qset *qset;

	qset = kzalloc(sizeof(*qset), GFP_KERNEL);
	if (!qset)
		goto err;

	/* pointer to each quanta */
	qset->data = kmalloc(sizeof(void *) * s->qset, GFP_KERNEL);
	if (!qset->data) {
		goto err;
	}
	return qset;
err:
	if (qset)
		kfree(qset);
	return ERR_PTR(-ENOMEM);
}

/* find the quantum set. */
static struct scull_qset *scull_follow(struct scull *s, const int item)
{
	struct scull_qset **dptr;
	int i;

	/* follow the data and allocate the appropriate qset */
	for (dptr = &s->data, i = 0; i <= item; dptr = &(*dptr)->next, i++) {
		if (!*dptr) {
			struct scull_qset *qset = alloc_qset(s);
			if (IS_ERR(qset))
				return qset;
			*dptr = qset;
		}
	}
	return *dptr;
}

static void scull_trim(struct scull *s)
{
	struct scull_qset *next, *dptr;
	int qset = s->qset;
	int i;

	pr_info("%s\n", __FUNCTION__);

	for (dptr = s->data; dptr; dptr = next) {
		if (dptr->data) {
			for (i = 0; i < qset; i++)
				kfree(dptr->data[i]);
			kfree(dptr->data);
			dptr->data = NULL;
		}
		next = dptr->next;
		kfree(dptr);
	}
	s->qset = scull_qset;
	s->data = NULL;
}

/* File operations. */
static int scull_open(struct inode *i, struct file *f)
{
	struct scull *s = container_of(i->i_cdev, struct scull, cdev);

	pr_info("%s\n", __FUNCTION__);

	/* for read/write */
	f->private_data = s;

	/* trim the device when it was opened write-only */
	if ((f->f_flags & O_ACCMODE) == O_WRONLY)
		scull_trim(s);

	return 0;
}

static ssize_t scull_read(struct file *f, char __user *buf, size_t len, loff_t *pos)
{
	struct scull *s = f->private_data;

	pr_info("%s\n", __FUNCTION__);

	if (down_interruptible(&s->lock))
		return -ERESTARTSYS;
	up(&s->lock);
	return 0;
}

static ssize_t scull_write(struct file *f, const char __user *buf, size_t len, loff_t *pos)
{
	struct scull *s = f->private_data;
	int quantum = s->quantum, qset = s->qset;
	int itemsize = quantum * qset;
	int item, rest, s_pos, q_pos;
	struct scull_qset *dptr;
	ssize_t ret = -ENOMEM;

	pr_info("%s\n", __FUNCTION__);

	if (down_interruptible(&s->lock))
		return -ERESTARTSYS;

	/* get the quantum set, qset index, and offset inside the quantum */
	item = (long)*pos / itemsize;
	rest = (long)*pos % itemsize;
	s_pos = rest / quantum;
	q_pos = rest % quantum;

	/* find the quantum set */
	dptr = scull_follow(s, item);
	if (IS_ERR(dptr))
		goto out;
out:
	up(&s->lock);
	return ret;
}

static int scull_release(struct inode *i, struct file *f)
{
	pr_info("%s\n", __FUNCTION__);
	return 0;
}

static struct file_operations scull_fops = {
	.owner = THIS_MODULE,
	.open = scull_open,
	.read = scull_read,
	.write = scull_write,
	.release = scull_release,
};

static void scull_initialize(struct scull *s, dev_t devt, const char *name)
{
	device_initialize(&s->dev);
	s->dev.devt = devt;
	s->dev.init_name = name;
	cdev_init(&s->cdev, &scull_fops);
	s->cdev.owner = THIS_MODULE;
	s->quantum = scull_quantum;
	s->qset = scull_qset;
	s->data = NULL;
	sema_init(&s->lock, 1);
}

static int __init scull_init(void)
{
	const int nr_dev = ARRAY_SIZE(sculls);
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
		dev_t devt = MKDEV(MAJOR(dev_base), MINOR(dev_base) + i);
		char name[SCULL_DEV_NAME_LEN];

		sprintf(name, SCULL_DEV_PREFIX "%d", i);
		scull_initialize(s, devt, name);

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
		/* free the quantum sets */
		scull_trim(s);
	}
	unregister_chrdev_region(dev_base, nr_dev);
}
module_exit(scull_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("LDD scull character device driver");
