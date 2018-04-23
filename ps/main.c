#include <linux/types.h>
#include <linux/list.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init_task.h>

static int __init ps_init(void)
{
	struct task_struct *task;
	struct list_head *list;

	pr_info("%s()\n", __FUNCTION__);

	/* list all the children */
	list_for_each(list, &current->children) {
		task = list_entry(list, struct task_struct, sibling);
		printk("%s[%d]\n", task->comm, task->pid);
	}

	/* look and print the origin of all the processes */
	for (task = current; task != &init_task; task = task->parent)
		;
	printk("%s[%d]\n", task->comm, task->pid);
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
MODULE_DESCRIPTION("listing all the process");
