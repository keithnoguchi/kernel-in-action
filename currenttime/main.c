/* SPDX-License-Identifier: GPL-2.0 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/time.h>
#include <linux/jiffies.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include "../ldd/ldd.h"

#define MAX_NR_CURRENTTIME	512

static struct ldd_device currenttime_devices[] = {
	{ .name = "currenttime0" },
	{ .name = "currenttime1" },
	{ /* sentry */ },
};

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
	struct ldd_device *d, *delete;
	int err;

	pr_info("%s\n", __FUNCTION__);

	err = currenttime_init_procfs();
	if (err)
		return err;

	/* sysfs based currenttime devices */
	for (d = currenttime_devices; d->name; d++) {
		err = register_ldd_device(d);
		if (err)
			goto unregister;
	}
	return 0;
unregister:
	delete = d;
	for (d = currenttime_devices; d != delete; d++)
		unregister_ldd_device(d);
	currenttime_exit_procfs();
	return err;
}
module_init(currenttime_init);

static void __exit currenttime_exit(void)
{
	struct ldd_device *d;

	pr_info("%s\n", __FUNCTION__);
	for (d = currenttime_devices; d->name; d++)
		unregister_ldd_device(d);
	currenttime_exit_procfs();
}
module_exit(currenttime_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("LDD's currenttime module");
