/* SPDX-License-Identifier: GPL-2.0 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/mutex.h>

#define NR_SLEEPY_DEV		4
#define SLEEPY_DEV_PREFIX	"sleep"
#define SLEEPY_DEV_NAME_LEN	(strlen(SLEEPY_DEV_PREFIX) + 2)

/* sleepy device descriptor */
static struct sleepy {
	struct mutex	lock;
	struct device	dev;
	struct cdev	cdev;
	int		ready;
} sleepys[NR_SLEEPY_DEV];

/* file operation methods */
static int sleepy_open(struct inode *i, struct file *f)
{
	struct sleepy *s = container_of(i->i_cdev, struct sleepy, cdev);
	pr_info("%s(%s)\n", __FUNCTION__, dev_name(&s->dev));
	f->private_data = s; /* for other methods */
	return 0;
}

static ssize_t sleepy_read(struct file *f, char __user *buf, size_t len, loff_t *pos)
{
	struct sleepy *s = f->private_data;
	pr_info("%s(%s)\n", __FUNCTION__, dev_name(&s->dev));
	mutex_lock_interruptible(&s->lock);
	mutex_unlock(&s->lock);
	return 0;
}

static ssize_t sleepy_write(struct file *f, const char __user *buf, size_t len, loff_t *pos)
{
	struct sleepy *s = f->private_data;
	pr_info("%s(%s)\n", __FUNCTION__, dev_name(&s->dev));
	mutex_lock_interruptible(&s->lock);
	mutex_unlock(&s->lock);
	return 0;
}

static int sleepy_release(struct inode *i, struct file *f)
{
	struct sleepy *s = f->private_data;
	pr_info("%s(%s)\n", __FUNCTION__, dev_name(&s->dev));
	/* just to be inline with sleepy_open() */
	f->private_data = NULL;
	return 0;
}

static const struct file_operations sleepy_ops = {
	.open = sleepy_open,
	.read = sleepy_read,
	.write = sleepy_write,
	.release = sleepy_release,
};

static void __init sleepy_initialize(struct sleepy *s, const dev_t dev_base, int i)
{
	char name[SLEEPY_DEV_NAME_LEN];

	device_initialize(&s->dev);
	s->dev.devt = MKDEV(MAJOR(dev_base), MINOR(dev_base) + i);
	sprintf(name, SLEEPY_DEV_PREFIX "%d", i);
	s->dev.init_name = name;
	cdev_init(&s->cdev, &sleepy_ops);
	s->cdev.owner = THIS_MODULE;
	s->ready = 0;
	mutex_init(&s->lock);
}

static int __init sleepy_init(void)
{
	const int nr_dev = ARRAY_SIZE(sleepys);
	dev_t dev_base;
	struct sleepy *s;
	int i, j;
	int err;

	pr_info("%s\n", __FUNCTION__);

	/* allocate the device number region for sleepy */
	err = alloc_chrdev_region(&dev_base, 0, nr_dev, SLEEPY_DEV_PREFIX);
	if (err)
		return err;

	/* create sleepy devices */
	for (s = sleepys, i = 0; i < nr_dev; s++, i++) {
		sleepy_initialize(s, dev_base, i);

		/* add cdev into the character device subsystem */
		err = cdev_device_add(&s->cdev, &s->dev);
		if (err)
			goto unregister;

		pr_info("%s[%d:%d]: added\n", dev_name(&s->dev),
			MAJOR(s->dev.devt), MINOR(s->dev.devt));
	}
	return 0;
unregister:
	/* only delete already added devices */
	for (s = sleepys, j = 0; j < i; s++, j++) {
		cdev_device_del(&s->cdev, &s->dev);
		pr_info("%s[%d:%d]: deleted\n", dev_name(&s->dev),
			MAJOR(s->dev.devt), MINOR(s->dev.devt));
	}
	unregister_chrdev_region(dev_base, nr_dev);
	return err;
}
module_init(sleepy_init);

static void __exit sleepy_exit(void)
{
	const int nr_dev = ARRAY_SIZE(sleepys);
	dev_t dev_base = sleepys[0].dev.devt;
	struct sleepy *s;
	int i;

	pr_info("%s\n", __FUNCTION__);

	for (s = sleepys, i = 0; i < nr_dev; s++, i++) {
		cdev_device_del(&s->cdev, &s->dev);
		pr_info("%s[%d:%d]: deleted\n", dev_name(&s->dev),
			MAJOR(s->dev.devt), MINOR(s->dev.devt));
	}
	unregister_chrdev_region(dev_base, nr_dev);
}
module_exit(sleepy_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("LDD's sleepy character driver");
