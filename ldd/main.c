/* SPDX-License-Identifier: GPL-2.0 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/string.h>

#include "ldd.h"

static void ldd_bus_release(struct device *d)
{
	pr_info("%s\n", __FUNCTION__);
}

static struct device ldd_bus = {
	.init_name = "ldd0",
	.release = ldd_bus_release,
};

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

static ssize_t show_version(struct device_driver *drv, char *buf)
{
	const struct ldd_driver *ldrv = to_ldd_driver(drv);

	sprintf(buf, "%s\n", ldrv->version);
	return strlen(buf);
}

int register_ldd_driver(struct ldd_driver *drv)
{
	int err;

	drv->driver.bus = &ldd_bus_type;
	err = driver_register(&drv->driver);
	if (err)
		return err;
	drv->version_attr.attr.name = "version";
	drv->version_attr.attr.mode = S_IRUGO;
	drv->version_attr.show = show_version;
	drv->version_attr.store = NULL;
	err = driver_create_file(&drv->driver, &drv->version_attr);
	if (err)
		goto unregister;
	return 0;
unregister:
	driver_unregister(&drv->driver);
	return err;
}
EXPORT_SYMBOL(register_ldd_driver);

void unregister_ldd_driver(struct ldd_driver *drv)
{
	driver_remove_file(&drv->driver, &drv->version_attr);
	driver_unregister(&drv->driver);
}
EXPORT_SYMBOL(unregister_ldd_driver);

static int __init ldd_init(void)
{
	int err;

	pr_info("%s\n", __FUNCTION__);

	err = bus_register(&ldd_bus_type);
	if (err)
		return err;

	err = device_register(&ldd_bus);
	if (err)
		goto unregister;

	return 0;
unregister:
	bus_unregister(&ldd_bus_type);
	return err;
}
module_init(ldd_init);

static void __exit ldd_exit(void)
{
	pr_info("%s\n", __FUNCTION__);
	device_unregister(&ldd_bus);
	bus_unregister(&ldd_bus_type);
}
module_exit(ldd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("LDD's Linux Device Model example");
