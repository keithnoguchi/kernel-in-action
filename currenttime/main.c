/* SPDX-License-Identifier: GPL-2.0 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/time.h>
#include <linux/jiffies.h>

#include "../ldd/ldd.h"

#define MAX_NR_CURRENTTIME	512

/* sysfs based currenttime device */
struct currenttime_device {
	struct ldd_device		dev;
	struct device_attribute		jiffies;
	struct device_attribute		jiffies_64;
	struct device_attribute		gettimeofday;
	struct device_attribute		current_kernel_time;
};

static ssize_t show_jiffies(struct device *dev, struct device_attribute *attr,
			    char *buf)
{
	return snprintf(buf, PAGE_SIZE, "0x%08lx\n", jiffies);
}

static ssize_t show_jiffies_64(struct device *dev, struct device_attribute *attr,
			       char *buf)
{
	return snprintf(buf, PAGE_SIZE, "0x%016llx\n", get_jiffies_64());
}

static ssize_t show_gettimeofday(struct device *dev, struct device_attribute *attr,
				 char *buf)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	return snprintf(buf, PAGE_SIZE, "%ld.%ld\n", tv.tv_sec, tv.tv_usec);
}

static ssize_t show_current_kernel_time(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct timespec ts = current_kernel_time();
	return snprintf(buf, PAGE_SIZE, "%ld.%ld\n", ts.tv_sec, ts.tv_nsec);
}

static struct currenttime_device currenttime_devices[] = {
	{
		.dev.name			= "currenttime0",
		.jiffies.attr.name		= "jiffies",
		.jiffies.attr.mode		= S_IRUGO,
		.jiffies.show			= show_jiffies,
		.jiffies_64.attr.name		= "jiffies_64",
		.jiffies_64.attr.mode		= S_IRUGO,
		.jiffies_64.show		= show_jiffies_64,
		.gettimeofday.attr.name		= "gettimeofday",
		.gettimeofday.attr.mode		= S_IRUGO,
		.gettimeofday.show		= show_gettimeofday,
		.current_kernel_time.attr.name	= "current_kernel_time",
		.current_kernel_time.attr.mode	= S_IRUGO,
		.current_kernel_time.show	= show_current_kernel_time,
	},
	{ /* sentry */ },
};

static int __init currenttime_init_sysfs(void)
{
	struct currenttime_device *d, *delete;
	int err;

	for (d = currenttime_devices; d->dev.name; d++) {
		err = register_ldd_device(&d->dev);
		if (err)
			return err;

		err = device_create_file(&d->dev.dev, &d->jiffies);
		if (err) {
			unregister_ldd_device(&d->dev);
			goto unregister;
		}
		err = device_create_file(&d->dev.dev, &d->jiffies_64);
		if (err) {
			device_remove_file(&d->dev.dev, &d->jiffies);
			unregister_ldd_device(&d->dev);
			goto unregister;
		}
		err = device_create_file(&d->dev.dev, &d->gettimeofday);
		if (err) {
			device_remove_file(&d->dev.dev, &d->jiffies_64);
			device_remove_file(&d->dev.dev, &d->jiffies);
			unregister_ldd_device(&d->dev);
			goto unregister;
		}
		err = device_create_file(&d->dev.dev, &d->current_kernel_time);
		if (err) {
			device_remove_file(&d->dev.dev, &d->gettimeofday);
			device_remove_file(&d->dev.dev, &d->jiffies_64);
			device_remove_file(&d->dev.dev, &d->jiffies);
			unregister_ldd_device(&d->dev);
			goto unregister;
		}
	}
	return 0;
unregister:
	delete = d;
	for (d = currenttime_devices; d != delete; d++)
		unregister_ldd_device(&d->dev);
	return err;
}

static void __exit currenttime_exit_sysfs(void)
{
	struct currenttime_device *d;

	for (d = currenttime_devices; d->dev.name; d++) {
		device_remove_file(&d->dev.dev, &d->current_kernel_time);
		device_remove_file(&d->dev.dev, &d->gettimeofday);
		device_remove_file(&d->dev.dev, &d->jiffies_64);
		device_remove_file(&d->dev.dev, &d->jiffies);
		unregister_ldd_device(&d->dev);
	}
}

static void *currenttime_procfs_ct_seq_start(struct seq_file *s, loff_t *pos)
{
	pr_info("%s(%lld)\n", __FUNCTION__, *pos);
	if (*pos >= MAX_NR_CURRENTTIME)
		return NULL;
	seq_printf(s, "jiffies\t\tjiffies_64\t\tdo_gettimeofday()\tcurrent_kernel_time()\n");
	return (void *)!NULL; /* we don't use void *v */
}

static void *currenttime_procfs_ct_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	pr_info("%s(%lld)\n", __FUNCTION__, *pos);
	(*pos)++;
	if (*pos >= MAX_NR_CURRENTTIME)
		return NULL;
	return v; /* just returns a dummy */
}

static void currenttime_procfs_ct_seq_stop(struct seq_file *s, void *v)
{
	pr_info("%s\n", __FUNCTION__);
	return;
}

static int currenttime_procfs_ct_seq_show(struct seq_file *s, void *v)
{
	struct timeval tv;
	struct timespec ts;

	pr_info("%s\n", __FUNCTION__);
	do_gettimeofday(&tv);
	ts = current_kernel_time();
	seq_printf(s, "0x%08lx\t0x%016llx\t%ld.%ld\t%ld.%ld\n", jiffies, get_jiffies_64(),
		   tv.tv_sec, tv.tv_usec, ts.tv_sec, ts.tv_nsec);

	return 0;
}

const static struct seq_operations currenttime_procfs_ct_seq_ops = {
	.start = currenttime_procfs_ct_seq_start,
	.next  = currenttime_procfs_ct_seq_next,
	.stop  = currenttime_procfs_ct_seq_stop,
	.show  = currenttime_procfs_ct_seq_show,
};

static int currenttime_procfs_ct_open(struct inode *i, struct file *f)
{
	pr_info("%s\n", __FUNCTION__);
	return seq_open(f, &currenttime_procfs_ct_seq_ops);
}

const static struct file_operations currenttime_procfs_ct_ops = {
	.owner   = THIS_MODULE,
	.open    = currenttime_procfs_ct_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release,
};

static int currenttime_init_procfs(void)
{
	struct proc_dir_entry *entry;

	entry = proc_create("currenttime", S_IRUGO, NULL, &currenttime_procfs_ct_ops);
	if (IS_ERR(entry))
		return PTR_ERR(entry);

	return 0;
}

static void currenttime_exit_procfs(void)
{
	remove_proc_entry("currenttime", NULL);
}

static int __init currenttime_init(void)
{
	int err;

	pr_info("%s\n", __FUNCTION__);

	err = currenttime_init_procfs();
	if (err)
		return err;

	/* sysfs based currenttime devices */
	err = currenttime_init_sysfs();
	if (err)
		goto unregister;

	return 0;
unregister:
	currenttime_exit_procfs();
	return err;
}
module_init(currenttime_init);

static void __exit currenttime_exit(void)
{
	pr_info("%s\n", __FUNCTION__);
	currenttime_exit_sysfs();
	currenttime_exit_procfs();
}
module_exit(currenttime_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("LDD's currenttime module");
