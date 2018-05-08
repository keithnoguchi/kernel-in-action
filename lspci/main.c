/* SPDX-License-Identifier: GPL-2.0 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/mod_devicetable.h>

const static struct pci_device_id ids[] = {
	{ PCI_DEVICE(PCI_ANY_ID, PCI_ANY_ID), }, /* match any device */
	{ 0, /* sentry */ },
};
MODULE_DEVICE_TABLE(pci, ids);

static int probe(struct pci_dev *dev, const struct pci_device_id *ids)
{
	pr_info("%s(%s)\n", __FUNCTION__, pci_name(dev));
	return 0;
}

static void remove(struct pci_dev *dev)
{
	pr_info("%s(%s)\n", __FUNCTION__, pci_name(dev));
}

static struct pci_driver lspci_driver = {
	.name     = "lspci",
	.id_table = ids,
	.probe    = probe,
	.remove   = remove,
};

static int __init lspci_init(void)
{
	int err;

	pr_info("%s\n", __FUNCTION__);
	err = pci_register_driver(&lspci_driver);
	if (err)
		return err;

	return 0;
}
module_init(lspci_init);

static void __exit lspci_exit(void)
{
	pr_info("%s\n", __FUNCTION__);
	pci_unregister_driver(&lspci_driver);
}
module_exit(lspci_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("Simple module to list the PCI devices");
