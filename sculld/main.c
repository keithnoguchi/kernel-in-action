/* SPDX-License-Identifier: GPL-2.0 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>

#include "../ldd/ldd.h"

#define SCULLD_DRIVER_VERSION	"1.0"
#define SCULLD_DRIVER_NAME	"sculld"

/* Sculld driver version and the name */
static const char *sculld_driver_version = SCULLD_DRIVER_VERSION;
static const char *sculld_driver_name = SCULLD_DRIVER_NAME;

/* sculld driver */
static struct ldd_driver sculld_driver = {
	.module = THIS_MODULE,
};

static struct ldd_device sculld_devices[] = {
	{ .name = "sculld0" },
	{ .name = "sculld1" },
	{ .name = "sculld2" },
	{ .name = "sculld3" },
	{ /* sentry */ },
};

static int __init sculld_init(void)
{
	struct ldd_device *d, *delete;
	int err;

	pr_info("%s\n", __FUNCTION__);

	/*
	 * allow to change the version and the name
	 * through the module parameter.
	 */
	sculld_driver.version = sculld_driver_version;
	sculld_driver.driver.name = sculld_driver_name;
	err = register_ldd_driver(&sculld_driver);
	if (err)
		return err;

	/* register sculld devices */
	for (d = sculld_devices; d->name; d++) {
		err = register_ldd_device(d);
		if (err)
			goto unregister;
	}
	return 0;
unregister:
	/*
	 * unregister devices before d, which holds the pointer to
	 * the failed device.
	 */
	for (delete = sculld_devices; d && delete != d; delete++)
		unregister_ldd_device(delete);
	unregister_ldd_driver(&sculld_driver);
	return err;
}
module_init(sculld_init);

static void __exit sculld_exit(void)
{
	struct ldd_device *d;

	pr_info("%s\n", __FUNCTION__);

	unregister_ldd_driver(&sculld_driver);
	for (d = sculld_devices; d->name; d++)
		unregister_ldd_device(d);
}
module_exit(sculld_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("LDD's scull driver under ldd virtual bus");
