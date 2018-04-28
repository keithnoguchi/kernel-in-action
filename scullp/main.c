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

#define NR_SCULLP_DEV			4
#define SCULLP_DEFAULT_DEBUG_STATUS	1
#define SCULLP_DEFAULT_BUFFER_SIZE	PAGE_SIZE
#define SCULLP_DEV_PREFIX		"scullp"
#define SCULLP_DEV_NAME_LEN		(strlen(SCULLP_DEV_PREFIX) + 2)

#define scullp_debug(fmt, ...)                                         \
	do {                                                           \
		if (is_scullp_debug())                                 \
			printk(KERN_INFO "%s[%s]: " pr_fmt(fmt) "\n",  \
			       module_name(THIS_MODULE), __FUNCTION__, \
                               ##__VA_ARGS__);                         \
	} while (0)

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
} scullps[NR_SCULLP_DEV];

/* module wide variable, to be controled by module parameters and sysfs */
static int debug = SCULLP_DEFAULT_DEBUG_STATUS;
static int buffer_size = SCULLP_DEFAULT_BUFFER_SIZE;
module_param(debug, int, S_IRUGO|S_IWUSR);
module_param(buffer_size, int, S_IRUGO);

static inline int is_scullp_debug(void)
{
	int is_debug;

	/* lock is required, as explained in linux/moduleparam.h */
	kernel_param_lock(THIS_MODULE);
	is_debug = debug;
	kernel_param_unlock(THIS_MODULE);

	return is_debug ? 1 : 0;
}

static inline int scullp_buffer_size(void)
{
	/* no lock required, as it's read only */
	return buffer_size;
}

/* how much data ready for read? */
static inline size_t readable_size(const struct scullp *s)
{
	if (s->readp == s->writep)
		return 0;
	return (s->writep + s->size - s->readp) % s->size;
}

static ssize_t scullp_read(struct file *f, char __user *buf, size_t len, loff_t *pos)
{
	struct scullp *s = f->private_data;
	DEFINE_WAIT(w);
	size_t size;
	int err;

	scullp_debug("reading from %s", dev_name(&s->dev));

	if (mutex_lock_interruptible(&s->lock))
		return -ERESTARTSYS;

	/* wait for buffer to be ready to read */
	err = 0;
	add_wait_queue(&s->inwq, &w);
	while ((size = readable_size(s)) <= 0) {
		prepare_to_wait_exclusive(&s->inwq, &w, TASK_INTERRUPTIBLE);
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
		err = 0; /* reset error before next try */
	}
	finish_wait(&s->inwq, &w);

	/* error happened while waiting for the buffer */
	if (err)
		goto out;

	/* adjust the length of the buffer to read */
	len = min(len, size);
	if (s->readp > s->writep)
		len = min(len, (size_t)(s->size - s->readp));

	/* copy to the user buffer */
	err = -EFAULT;
	if (copy_to_user(buf, s->buffer + s->readp, len))
		goto out;

	/* adjust the read position and the return value */
	s->readp += len;
	if (s->readp == s->size)
		s->readp = 0;
	err = len;

	/* finally, wake up the writer */
	wake_up_interruptible(&s->ewq);
out:
	mutex_unlock(&s->lock);
	return err;
}

/* how much space available for write? */
static inline size_t writable_size(const struct scullp *s)
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
	size_t size;
	int err;

	scullp_debug("writing on %s", dev_name(&s->dev));
	if (mutex_lock_interruptible(&s->lock))
		return -ERESTARTSYS;

	/* wait for the buffer to be ready for write */
	err = 0;
	add_wait_queue(&s->ewq, &w);
	while ((size = writable_size(s)) <= 0) {
		prepare_to_wait_exclusive(&s->ewq, &w, TASK_INTERRUPTIBLE);
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
		err = 0; /* reset error before next try */
	}
	finish_wait(&s->ewq, &w);

	/* error happened while waiting for the buffer */
	if (err)
		goto out;

	/* adjust the length of the buffer to write */
	len = min(len, size);
	if (s->writep >= s->readp)
		len = min(len, (size_t)(s->size - s->writep));

	/* copy from the user space */
	err = -EFAULT;
	if (copy_from_user((s->buffer + s->writep), buf, len))
		goto out;

	/* update the write position and the return value */
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

	scullp_debug("opening %s", dev_name(&s->dev));
	f->private_data = s;

	return 0;
}

static int scullp_release(struct inode *i, struct file *f)
{
	struct scullp *s = f->private_data;

	scullp_debug("releasing %s", dev_name(&s->dev));
	f->private_data = NULL;

	return 0;
}

static const struct file_operations scullp_ops __initconst = {
	.read = scullp_read,
	.write = scullp_write,
	.open = scullp_open,
	.release = scullp_release,
};

static int __init scullp_initialize(struct scullp *s, const dev_t dev_base, int i)
{
	char name[SCULLP_DEV_NAME_LEN];

	memset(&s->dev, 0, sizeof(s->dev));
	device_initialize(&s->dev);
	s->dev.devt = MKDEV(MAJOR(dev_base), MINOR(dev_base)+i);
	sprintf(name, SCULLP_DEV_PREFIX "%d", i);
	s->dev.init_name = name;
	cdev_init(&s->cdev, &scullp_ops);
	s->cdev.owner = THIS_MODULE;
	init_waitqueue_head(&s->ewq);
	init_waitqueue_head(&s->inwq);
	mutex_init(&s->lock);
	s->size = scullp_buffer_size();
	s->buffer = kzalloc(s->size, GFP_KERNEL);
	if (!s->buffer)
		return -ENOMEM;
	s->readp = s->writep = 0;

	return 0;
}

static void scullp_terminate(struct scullp *s)
{
	scullp_debug("deleting %s[%d:%d]", dev_name(&s->dev),
		     MAJOR(s->dev.devt), MINOR(s->dev.devt));
	cdev_device_del(&s->cdev, &s->dev);
	if (s->buffer)
		kfree(s->buffer);
	s->buffer = NULL;
	s->readp = s->writep = s->size = 0;
}

static int __init scullp_init(void)
{
	int nr_dev = ARRAY_SIZE(scullps);
	struct scullp *s;
	dev_t dev_base;
	int i, j;
	int err;

	scullp_debug();

	/* allocate the base device number */
	err = alloc_chrdev_region(&dev_base, 0, nr_dev, SCULLP_DEV_PREFIX);
	if (err)
		return err;

	/* register the char devices. */
	s = scullps;
	for (i = 0; i < nr_dev; i++, s++) {
		err = scullp_initialize(s, dev_base, i);
		if (err)
			goto unregister;
		err = cdev_device_add(&s->cdev, &s->dev);
		if (err)
			goto unregister;
		scullp_debug("added %s[%d:%d]", dev_name(&s->dev),
			     MAJOR(s->dev.devt), MINOR(s->dev.devt));
	}
	return 0;
unregister:
	s = scullps;
	for (j = 0; j < i; j++, s++)
		scullp_terminate(s);
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

	scullp_debug();
	for (i = 0; i < nr_dev; i++, s++)
		scullp_terminate(s);
	unregister_chrdev_region(dev_base, nr_dev);
}
module_exit(scullp_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("Scull pipe device driver");
