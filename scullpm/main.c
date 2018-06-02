/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>

#include "../ldd/ldd.h"

#define SCULLPM_DRIVER_NAME	"scullpm"
#define SCULLPM_DRIVER_VERSION	"1.0.0"

static struct scullpm_driver {
	struct ldd_driver	ldd;
} scullpm = {
	.ldd.module		= THIS_MODULE,
	.ldd.version		= SCULLPM_DRIVER_VERSION,
	.ldd.driver.name	= SCULLPM_DRIVER_NAME,
};

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
	pr_info("%s\n", __FUNCTION__);
	return register_driver(&scullpm);
}
module_init(init);

static void __exit cleanup(void)
{
	pr_info("%s\n", __FUNCTION__);
	unregister_driver(&scullpm);
}
module_exit(cleanup);

MODULE_LICENSE("GPL-2.0");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("scullpm: Page based scull");
