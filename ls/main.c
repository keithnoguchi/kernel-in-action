/* SPDX-License-Identifier: GPL-2.0 */

#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>

static int __init ls_init(void)
{
	struct task_struct *t;

	/* print all the processes */
	for_each_process(t)
		printk("%s[%d]\n", t->comm, t->pid);

	return 0;
}
module_init(ls_init);

static void __exit ls_exit(void)
{
	pr_info("%s()\n", __FUNCTION__);
}
module_exit(ls_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("listing all the processes");
