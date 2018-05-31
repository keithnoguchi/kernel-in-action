/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/device.h>

#include "../ldd/ldd.h"

#define SCULLC_DRIVER_VERSION		"1.0"
#define SCULLC_DRIVER_NAME		"scullc"
#define SCULLC_DEFAULT_QUANTUM_SIZE	PAGE_SIZE

static const char *driver_version = SCULLC_DRIVER_VERSION;
static const char *driver_name = SCULLC_DRIVER_NAME;
static int quantum_size = SCULLC_DEFAULT_QUANTUM_SIZE;
module_param(quantum_size, int, S_IRUGO);

static struct ldd_driver scullc_driver = {
	.module = THIS_MODULE,
};

static struct ldd_device scullc_devices[] = {
	{ .name = SCULLC_DRIVER_NAME "0" },
	{ .name = SCULLC_DRIVER_NAME "1" },
	{ .name = SCULLC_DRIVER_NAME "2" },
	{ .name = SCULLC_DRIVER_NAME "3" },
	{ /* sentry */ },
};

/* kmem cache */
static struct kmem_cache *quantum_cache;

static int __init scullc_init_module(void)
{
	struct ldd_device *d, *del;
	int err;

	pr_info("%s\n", __FUNCTION__);

	quantum_cache = kmem_cache_create("scullc_quantum", quantum_size,
					  0, SLAB_HWCACHE_ALIGN, NULL);
	if (!quantum_cache)
		return -ENOMEM;

	scullc_driver.version = driver_version;
	scullc_driver.driver.name = driver_name;
	err = register_ldd_driver(&scullc_driver);
	if (err)
		goto destroy_cache;

	for (d = scullc_devices; d->name; d++) {
		err = register_ldd_device(d);
		if (err)
			goto unregister;
	}
	return 0;
unregister:
	for (del = scullc_devices; del != d; del++)
		unregister_ldd_device(del);
	unregister_ldd_driver(&scullc_driver);
destroy_cache:
	if (quantum_cache)
		kmem_cache_destroy(quantum_cache);
	return err;
}
module_init(scullc_init_module);

static void __exit scullc_exit_module(void)
{
	struct ldd_device *d;

	pr_info("%s\n", __FUNCTION__);

	for (d = scullc_devices; d->name; d++)
		unregister_ldd_device(d);
	unregister_ldd_driver(&scullc_driver);
	if (quantum_cache)
		kmem_cache_destroy(quantum_cache);
}
module_exit(scullc_exit_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("kmem_cache_alloc() example");
