#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <asm/param.h>

/* khellod kthread. */
static struct task_struct *khellod;

/* Print out the hello world every 1 sec. */
#define KHELLOD_INTERVAL                (1*HZ)
static unsigned long khellod_interval = KHELLOD_INTERVAL;

static int hellod(void *unused)
{
	/* print hello world! every khellod_interval forever */
	for (;;) {
		printk("hello world!\n");

		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(khellod_interval);
		if (kthread_should_stop())
			break;
	}
	return 0;
}

static int __init khellod_init(void)
{
	pr_info("%s\n", __FUNCTION__);

	khellod = kthread_run(hellod, NULL, "khellod");
	if (IS_ERR(khellod))
		return PTR_ERR(khellod);

	return 0;
}
module_init(khellod_init);

static void __exit khellod_exit(void)
{
	pr_info("%s\n", __FUNCTION__);

	if (khellod)
		kthread_stop(khellod);
}
module_exit(khellod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("hello world kernel thread example");
