/* SPDX-License-Identifier: GPL-2.0 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/uaccess.h>
#include <asm/page.h>

#define NR_SCULL_PIPE_DEV		4
#define SCULL_PIPE_BUFFER_SIZ		PAGE_SIZE
#define SCULL_PIPE_DEV_PREFIX		"scullp"
#define SCULL_PIPE_DEV_NAME_LEN		(strlen(SCULL_PIPE_DEV_PREFIX) + 2)

/* scull pipe device descriptor */
struct scullp {
	struct mutex			lock;
	wait_queue_head_t		ewq;
	wait_queue_head_t		inwq;
	char				*buffer;
	size_t				size;
	size_t				readp;
	size_t				writep;
	struct device			dev;
	struct cdev			cdev;
} scullps[NR_SCULL_PIPE_DEV];

static ssize_t scullp_read(struct file *f, char __user *buf, size_t len, loff_t *pos)
{
	struct scullp *s = f->private_data;

	pr_info("%s(%s)\n", __FUNCTION__, dev_name(&s->dev));
	if (mutex_lock_interruptible(&s->lock))
		return -ERESTARTSYS;
	mutex_unlock(&s->lock);
	return -ENOTTY;
}

/* get the writable size in the buffer */
static inline size_t writable_size(struct scullp *s)
{
	size_t size;

	if (s->readp == s->writep)
		size = s->size;
	else
		size = (s->readp + s->size - s->writep) % s->size;

	/* -1 to indicate no more writable buffer. */
	return size - 1;
}

static ssize_t scullp_write(struct file *f, const char __user *buf, size_t len, loff_t *pos)
{
	struct scullp *s = f->private_data;
	DEFINE_WAIT(w);
	int err = 0;
	size_t size;

	pr_info("%s(%s)\n", __FUNCTION__, dev_name(&s->dev));
	if (mutex_lock_interruptible(&s->lock))
		return -ERESTARTSYS;

	/* wait for the buffer is ready to write */
	add_wait_queue(&s->ewq, &w);
	while ((size = writable_size(s)) <= 0) {
		prepare_to_wait(&s->ewq, &w, TASK_INTERRUPTIBLE);
		err = -EAGAIN;
		if (f->f_flags & O_NONBLOCK)
			break;
		err = -EINTR;
		if (signal_pending(current))
			break;
		mutex_unlock(&s->lock);
		schedule();
		if (mutex_lock_interruptible(&s->lock))
			return -ERESTARTSYS;
	}
	finish_wait(&s->ewq, &w);

	/* error happened while waiting for the buffer */
	if (err)
		goto out;

	/* adjust the length of the buffer to copy */
	len = min(len, size);
	if (s->writep >= s->readp)
		len = min(len, (size_t)(s->size - s->writep));
	else
		len = min(len, (size_t)(s->readp - s->writep - 1));

	/* copy from the user space */
	err = -EFAULT;
	if (copy_from_user((s->buffer + s->writep), buf, len))
		goto out;

	/* update the next write position and the return value */
	s->writep += len;
	if (s->writep == s->size)
		s->writep = 0;
	err = len;

	/* finally, wake up the reader */
	wake_up_interruptible(&s->inwq);
out:
	mutex_unlock(&s->lock);
	return err;
}

static int scullp_open(struct inode *i, struct file *f)
{
	struct scullp *s = container_of(i->i_cdev, struct scullp, cdev);

	pr_info("%s(%s)\n", __FUNCTION__, dev_name(&s->dev));

	s->size = SCULL_PIPE_BUFFER_SIZ;
	s->buffer = kzalloc(s->size, GFP_KERNEL);
	if (!s->buffer)
		return -ENOMEM;
	s->readp = s->writep = 0;
	f->private_data = s;

	return 0;
}

static int scullp_release(struct inode *i, struct file *f)
{
	struct scullp *s = f->private_data;

	pr_info("%s(%s)\n", __FUNCTION__, dev_name(&s->dev));
	if (s->buffer)
		kfree(s->buffer);
	s->buffer = NULL;
	s->readp = s->writep = s->size = 0;
	f->private_data = NULL;

	return 0;
}

static const struct file_operations scullp_ops = {
	.read = scullp_read,
	.write = scullp_write,
	.open = scullp_open,
	.release = scullp_release,
};

static void __init scullp_initialize(struct scullp *s, const dev_t dev_base, int i)
{
	char name[SCULL_PIPE_DEV_NAME_LEN];

	memset(&s->dev, 0, sizeof(s->dev));
	device_initialize(&s->dev);
	s->dev.devt = MKDEV(MAJOR(dev_base), MINOR(dev_base)+i);
	sprintf(name, SCULL_PIPE_DEV_PREFIX "%d", i);
	s->dev.init_name = name;
	cdev_init(&s->cdev, &scullp_ops);
	s->cdev.owner = THIS_MODULE;
	init_waitqueue_head(&s->ewq);
	init_waitqueue_head(&s->inwq);
	mutex_init(&s->lock);
}

static int __init scullp_init(void)
{
	int nr_dev = ARRAY_SIZE(scullps);
	struct scullp *s = scullps;
	dev_t dev_base;
	int i, j;
	int err;

	pr_info("%s\n", __FUNCTION__);

	/* allocate the base device number */
	err = alloc_chrdev_region(&dev_base, 0, nr_dev, SCULL_PIPE_DEV_PREFIX);
	if (err)
		return err;

	/* register the char devices. */
	for (i = 0; i < nr_dev; i++, s++) {
		scullp_initialize(s, dev_base, i);
		err = cdev_device_add(&s->cdev, &s->dev);
		if (err)
			goto unregister;
		pr_info("%s[%d:%d]: added\n", dev_name(&s->dev),
			MAJOR(s->dev.devt), MINOR(s->dev.devt));
	}
	return 0;
unregister:
	for (j = 0; j < i; j++)
		cdev_device_del(&s->cdev, &s->dev);
	unregister_chrdev_region(dev_base, nr_dev);
	return err;
}
module_init(scullp_init);

static void __exit scullp_exit(void)
{
	int nr_dev = ARRAY_SIZE(scullps);
	struct scullp *s = scullps;
	dev_t dev_base = s->dev.devt;
	int i;

	pr_info("%s\n", __FUNCTION__);
	for (i = 0; i < nr_dev; i++, s++) {
		cdev_device_del(&s->cdev, &s->dev);
		pr_info("%s[%d:%d]: deleted\n", dev_name(&s->dev),
			MAJOR(s->dev.devt), MINOR(s->dev.devt));
	}
	unregister_chrdev_region(dev_base, nr_dev);
}
module_exit(scullp_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("Scull pipe device driver");
