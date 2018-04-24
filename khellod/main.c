#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>

static int __init khellod_init(void)
{
	pr_info("%s\n", __FUNCTION__);
	return 0;
}
module_init(khellod_init);

static void __exit khellod_exit(void)
{
	pr_info("%s\n", __FUNCTION__);
}
module_exit(khellod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("hello world kernel thread example");
