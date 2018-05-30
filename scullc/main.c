/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

#define SCULLC_DEFAULT_QUANTUM_SIZE	PAGE_SIZE

static struct kmem_cache *quantum_cache;
static int quantum = SCULLC_DEFAULT_QUANTUM_SIZE;
module_param(quantum, int, S_IRUGO);

static int __init scullc_init_module(void)
{
	pr_info("%s\n", __FUNCTION__);

	quantum_cache = kmem_cache_create("scullc_quantum", quantum,
					  0, SLAB_HWCACHE_ALIGN, NULL);
	if (!quantum_cache)
		return -ENOMEM;

	return 0;
}
module_init(scullc_init_module);

static void __exit scullc_exit_module(void)
{
	pr_info("%s\n", __FUNCTION__);

	if (quantum_cache)
		kmem_cache_destroy(quantum_cache);
}
module_exit(scullc_exit_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("kmem_cache_alloc() example");
