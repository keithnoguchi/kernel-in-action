/* SPDX-License-Identifier: GPL-2.0 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/err.h>

/* khellod kthread. */
static struct task_struct *khellod;
static char *msg = "Hello world!";

/* Print out the hello world every 1 sec. */
#define HELLO_INTERVAL                (1*HZ)
static unsigned long hello_interval = HELLO_INTERVAL;

static int hellod(void *data)
{
	const char *msg = data;

	/* print hello world! every hello_interval forever */
	for (;;) {
		printk("%s\n", msg);

		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(hello_interval);
		if (kthread_should_stop())
			break;
	}
	return 0;
}

static int __init hello_init(void)
{
	pr_info("%s\n", __FUNCTION__);

	khellod = kthread_run(hellod, msg, "khellod");
	if (IS_ERR(khellod))
		return PTR_ERR(khellod);

	return 0;
}
module_init(hello_init);

static void __exit hello_exit(void)
{
	pr_info("%s\n", __FUNCTION__);

	if (khellod)
		kthread_stop(khellod);
}
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("hello world kernel thread");
