/* SPDX-License-Identifier: GPL-2.0 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/jiffies.h>
#include <linux/time.h>

#define MAX_NR_CURRENTTIME	512

static void *jit_procfs_ct_seq_start(struct seq_file *s, loff_t *pos)
{
	pr_info("%s(%lld)\n", __FUNCTION__, *pos);
	if (*pos >= MAX_NR_CURRENTTIME)
		return NULL;
	seq_printf(s, "jiffies\t\tjiffies_64\t\tdo_gettimeofday()\tcurrent_kernel_time()\n");
	return (void *)!NULL; /* just a dummy */
}

static void *jit_procfs_ct_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	pr_info("%s(%lld)\n", __FUNCTION__, *pos);
	(*pos)++;
	if (*pos >= MAX_NR_CURRENTTIME)
		return NULL;
	return (void *)s;
}

static void jit_procfs_ct_seq_stop(struct seq_file *s, void *v)
{
	pr_info("%s\n", __FUNCTION__);
	return;
}

static int jit_procfs_ct_seq_show(struct seq_file *s, void *v)
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

const static struct seq_operations jit_procfs_ct_seq_ops = {
	.start = jit_procfs_ct_seq_start,
	.next  = jit_procfs_ct_seq_next,
	.stop  = jit_procfs_ct_seq_stop,
	.show  = jit_procfs_ct_seq_show,
};

static int jit_procfs_ct_open(struct inode *i, struct file *f)
{
	pr_info("%s\n", __FUNCTION__);
	return seq_open(f, &jit_procfs_ct_seq_ops);
}

const static struct file_operations jit_procfs_ct_ops = {
	.owner   = THIS_MODULE,
	.open    = jit_procfs_ct_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release,
};

static int jit_init_procfs(void)
{
	struct proc_dir_entry *entry;

	entry = proc_create("currenttime", S_IRUGO, NULL, &jit_procfs_ct_ops);
	if (IS_ERR(entry))
		return PTR_ERR(entry);

	return 0;
}

static void jit_exit_procfs(void)
{
	remove_proc_entry("currenttime", NULL);
}

static int __init jit_init(void)
{
	int err;

	pr_info("%s\n", __FUNCTION__);

	err = jit_init_procfs();
	if (err)
		return err;

	return 0;
}
module_init(jit_init);

static void __exit jit_exit(void)
{
	pr_info("%s\n", __FUNCTION__);
	jit_exit_procfs();
}
module_exit(jit_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("LDD's Just-In-Time module");
