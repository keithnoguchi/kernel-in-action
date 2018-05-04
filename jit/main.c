/* SPDX-License-Identifier: GPL-2.0 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/err.h>

static struct file_operations jit_currenttime_ops = {
	.owner = THIS_MODULE,
	//.open = jit_currenttime_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

static int __init jit_init(void)
{
	struct proc_dir_entry *entry;

	pr_info("%s\n", __FUNCTION__);

	entry = proc_create("currenttime", S_IRUGO, NULL, &jit_currenttime_ops);
	if (IS_ERR(entry))
		return PTR_ERR(entry);

	return 0;
}
module_init(jit_init);

static void __exit jit_exit(void)
{
	pr_info("%s\n", __FUNCTION__);
	remove_proc_entry("currenttime", NULL);
}
module_exit(jit_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("LDD's Just-In-Time module");
