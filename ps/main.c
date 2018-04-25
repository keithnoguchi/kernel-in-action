/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>

static int __init ps_init(void)
{
	struct task_struct *task;

	/* print all the processes */
	for_each_process(task) {
		printk("%s[%d]\n", task->comm, task->pid);
	}
	return 0;
}
module_init(ps_init);

static void __exit ps_exit(void)
{
	pr_info("%s()\n", __FUNCTION__);
}
module_exit(ps_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("listing all the processes");
