/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _LDD_H
#define _LDD_H

/* ldd driver */
struct ldd_driver {
	const char		*version;
	struct module		*module;
	struct device_driver	driver;
	struct driver_attribute	version_attr;
};
#define to_ldd_driver(_drv)	container_of(_drv, struct ldd_driver, driver)

int register_ldd_driver(struct ldd_driver *drv);
void unregister_ldd_driver(struct ldd_driver *drv);

#endif /* _LDD_H */
