/* SPDX-License-Identifier: GPL-2.0 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

enum jit_procfs_ct_type {
	JIT_PROCFS_CT_NONE = 0,
	JIT_PROCFS_CT_JIFFIES,
	JIT_PROCFS_CT_JIFFIES64,
	JIT_PROCFS_CT_DO_GETTIMEOFDAY,
	JIT_PROCFS_CT_CURRENT_KERNEL_TIME,
	JIT_PROCFS_CT_MAX,
};

static void *jit_procfs_ct_seq_start(struct seq_file *s, loff_t *pos)
{
	pr_info("%s\n", __FUNCTION__);
	*pos = JIT_PROCFS_CT_JIFFIES;
	return (void *)JIT_PROCFS_CT_JIFFIES;
}

static void *jit_procfs_ct_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	enum jit_procfs_ct_type type = (enum jit_procfs_ct_type)v;
	if ((*pos) == JIT_PROCFS_CT_MAX)
		return NULL;
	type++; (*pos)++;
	pr_info("%s(%d)\n", __FUNCTION__, type);
	return (void *)type;
}

static void jit_procfs_ct_seq_stop(struct seq_file *s, void *v)
{
	enum jit_procfs_ct_type type = (enum jit_procfs_ct_type)v;
	pr_info("%s(%d)\n", __FUNCTION__, type);
	return;
}

static int jit_procfs_ct_seq_show(struct seq_file *s, void *v)
{
	enum jit_procfs_ct_type type = (enum jit_procfs_ct_type)v;
	pr_info("%s(%d)\n", __FUNCTION__, type);
	seq_printf(s, "JIT currenttime type: %d\n", type);
	return 0;
}

const static struct seq_operations jit_procfs_ct_seq_ops = {
	.start = jit_procfs_ct_seq_start,
	.next = jit_procfs_ct_seq_next,
	.stop = jit_procfs_ct_seq_stop,
	.show = jit_procfs_ct_seq_show,
};

static int jit_procfs_ct_open(struct inode *i, struct file *f)
{
	pr_info("%s\n", __FUNCTION__);
	return seq_open(f, &jit_procfs_ct_seq_ops);
}

const static struct file_operations jit_procfs_ct_ops = {
	.owner = THIS_MODULE,
	.open = jit_procfs_ct_open,
	.read = seq_read,
	.llseek = seq_lseek,
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
