#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int __init ps_init(void)
{
	pr_info("%s()\n", __FUNCTION__);
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
