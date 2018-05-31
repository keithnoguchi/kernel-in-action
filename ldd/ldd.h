/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _LDD_H
#define _LDD_H

/* ldd device */
struct ldd_device {
	const char		*name;
	struct ldd_driver	*driver;
	struct device		dev;
};
#define to_ldd_device(_dev)	container_of(_dev, struct lddd_device, dev)

/* ldd device driver */
struct ldd_driver {
	const char		*version;
	struct module		*module;
	struct device_driver	driver;
	struct driver_attribute	version_attr;
};
#define to_ldd_driver(_drv)	container_of(_drv, struct ldd_driver, driver)

static inline const char *ldd_dev_name(const struct ldd_device *dev)
{
	return dev_name(&dev->dev);
}
int register_ldd_device(struct ldd_device *dev);
void unregister_ldd_device(struct ldd_device *dev);
int register_ldd_driver(struct ldd_driver *drv);
void unregister_ldd_driver(struct ldd_driver *drv);

#endif /* _LDD_H */
