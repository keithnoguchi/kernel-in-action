/* SPDX-License-Identifier: GPL-2.0 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>

static int ldd_match(struct device *dev, struct device_driver *drv)
{
	pr_info("%s\n", __FUNCTION__);
	return 0;
}

static int ldd_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	pr_info("%s\n", __FUNCTION__);
	return 0;
}

static struct bus_type ldd_bus_type = {
	.name = "ldd",
	.match = ldd_match,
	.uevent = ldd_uevent,
};

static int __init sculld_init(void)
{
	int err;

	pr_info("%s\n", __FUNCTION__);

	err = bus_register(&ldd_bus_type);
	if (err)
		return err;

	return 0;
}
module_init(sculld_init);

static void __exit sculld_exit(void)
{
	pr_info("%s\n", __FUNCTION__);
	bus_unregister(&ldd_bus_type);
}
module_exit(sculld_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("LDD's Linux Device Model example");
