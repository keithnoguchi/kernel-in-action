/* SPDX-License-Identifier: GPL-2.0 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>

#define NR_SCULL_PIPE_DEV	4
#define SCULL_PIPE_DEV_PREFIX	"scullp"
#define SCULL_PIPE_DEV_NAME_LEN	(strlen(SCULL_PIPE_DEV_PREFIX) + 2)

/* scull pipe device descriptor */
struct scullp {
	struct device		dev;
	struct cdev		cdev;
} scullps[NR_SCULL_PIPE_DEV];

static int scullp_open(struct inode *i, struct file *f)
{
	struct scullp *s = container_of(i->i_cdev, struct scullp, cdev);

	pr_info("%s(%s)\n", __FUNCTION__, dev_name(&s->dev));
	f->private_data = s;

	return 0;
}

static ssize_t scullp_read(struct file *f, char __user *buf, size_t len, loff_t *pos)
{
	return -ENOTTY;
}

static ssize_t scullp_write(struct file *f, const char __user *buf, size_t len, loff_t *pos)
{
	return -ENOTTY;
}

static int scullp_release(struct inode *i, struct file *f)
{
	struct scullp *s = f->private_data;

	pr_info("%s(%s)\n", __FUNCTION__, dev_name(&s->dev));
	f->private_data = NULL;

	return 0;
}

static const struct file_operations scullp_ops = {
	.open = scullp_open,
	.read = scullp_read,
	.write = scullp_write,
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
