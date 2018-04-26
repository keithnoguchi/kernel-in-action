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
#include <linux/uaccess.h>
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
	int			size;
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

	/* pointer to each quantum */
	qset->data = kzalloc(sizeof(void *) * s->qset, GFP_KERNEL);
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
static struct scull_qset *find_qset(struct scull *s, int item)
{
	struct scull_qset *dptr;
	int i;

	for (i = 0, dptr = s->data; i < item; i++, dptr = dptr->next)
		if (!dptr)
			break;
	return dptr;
}

/* get the quantum set. */
static struct scull_qset *get_qset(struct scull *s, int item)
{
	struct scull_qset **dptr;
	int i;

	/* follow the data and allocate the appropriate qset */
	for (i = 0, dptr = &s->data; i <= item; i++, dptr = &(*dptr)->next) {
		if (!*dptr) {
			struct scull_qset *qset = alloc_qset(s);
			if (IS_ERR(qset))
				return qset;
			*dptr = qset;
		}
		if (item == i)
			return *dptr;
	}
	return *dptr;
}

/* trim the quantum set. */
static void trim_qset(struct scull *s)
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
	s->size = 0;
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
		trim_qset(s);

	return 0;
}

static ssize_t scull_read(struct file *f, char __user *buf, size_t len, loff_t *pos)
{
	struct scull *s = f->private_data;
	int quantum = s->quantum, qset = s->qset;
	int itemsize = quantum * qset;
	int item, rest, s_pos, q_pos;
	struct scull_qset *dptr;
	ssize_t ret = 0;

	pr_info("%s\n", __FUNCTION__);

	if (down_interruptible(&s->lock))
		return -ERESTARTSYS;

	if (*pos >= s->size)
		goto out;
	if (*pos + len > s->size)
		len = s->size - *pos;

	/* find the quantum set */
	item = (long)*pos / itemsize;
	rest = (long)*pos % itemsize;
	s_pos = rest / quantum;
	q_pos = rest % quantum;
	dptr = find_qset(s, item);
	if (!dptr || !dptr->data || !dptr->data[s_pos])
		goto out;

	/* read only up to the end of this quantum */
	if (len > quantum - q_pos)
		len = quantum - q_pos;

	if (copy_to_user(buf, dptr->data[s_pos]+q_pos, len)) {
		ret = -EFAULT;
		goto out;
	}
	*pos += len;
	ret = len;
out:
	up(&s->lock);
	return ret;
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

	/* get the quantum set */
	dptr = get_qset(s, item);
	if (IS_ERR_OR_NULL(dptr))
		goto out;

	/* find the quantum */
	if (!dptr->data[s_pos]) {
		dptr->data[s_pos] = kzalloc(s->quantum, GFP_KERNEL);
		if (!dptr->data[s_pos])
			goto out;
	}

	/* write only up to the end of this quantum */
	if (len > quantum - q_pos)
		len = quantum - q_pos;

	/* copy data to the device */
	if (copy_from_user(dptr->data[s_pos]+q_pos, buf, len)) {
		ret = -EFAULT;
		goto out;
	}
	*pos += len;
	ret = len;

	/* update the current size */
	if (s->size < *pos)
		s->size = *pos;
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
	s->size = 0;
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
		trim_qset(s);
	}
	unregister_chrdev_region(dev_base, nr_dev);
}
module_exit(scull_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("LDD scull character device driver");
