/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>

#include "../ldd/ldd.h"

#define SCULLPM_DRIVER_NAME	"scullpm"
#define SCULLPM_DRIVER_VERSION	"1.0.0"
#define SCULLPM_DEVICE_PREFIX	SCULLPM_DRIVER_NAME

/* driver */
static struct scullpm_driver {
	struct ldd_driver	ldd;
} scullpm = {
	.ldd.module		= THIS_MODULE,
	.ldd.version		= SCULLPM_DRIVER_VERSION,
	.ldd.driver.name	= SCULLPM_DRIVER_NAME,
};

/* devices */
static struct scullpm_device {
	struct ldd_device	ldd;
} devices[] = {
	{ .ldd.name	= SCULLPM_DEVICE_PREFIX "0" },
	{ .ldd.name	= SCULLPM_DEVICE_PREFIX "1" },
	{ /* sentinel */ },
};

static int register_device(struct scullpm_device *d)
{
	return register_ldd_device(&d->ldd);
}

static void unregister_device(struct scullpm_device *d)
{
	unregister_ldd_device(&d->ldd);
}

static int register_driver(struct scullpm_driver *drv)
{
	return register_ldd_driver(&drv->ldd);
}

static void unregister_driver(struct scullpm_driver *drv)
{
	unregister_ldd_driver(&drv->ldd);
}

static int __init init(void)
{
	struct scullpm_device *d, *del;
	int err;

	pr_info("%s\n", __FUNCTION__);
	err = register_driver(&scullpm);
	if (err)
		return err;

	for (d = devices; d->ldd.name; d++) {
		err = register_device(d);
		if (err)
			goto unregister;
	}
	return 0;
unregister:
	for (del = devices; del != d; del++)
		unregister_device(del);
	unregister_driver(&scullpm);
	return err;
}
module_init(init);

static void __exit cleanup(void)
{
	struct scullpm_device *d;

	pr_info("%s\n", __FUNCTION__);
	for (d = devices; d->ldd.name; d++)
		unregister_device(d);
	unregister_driver(&scullpm);
}
module_exit(cleanup);

MODULE_LICENSE("GPL-2.0");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("scullpm: Page based scull");
