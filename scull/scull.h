/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _KERNEL_IN_ACTION_SCULL_SCULL_H
#define _KERNEL_IN_ACTION_SCULL_SCULL_H

#include <linux/ioctl.h>

/* temporaly magic number. */
#define SCULL_IOC_MAGIC			0xff

#define SCULL_IOCRESET			_IO(SCULL_IOC_MAGIC, 0)

/*
 * S means "Set" through a ptr.
 * T means "Tell" directory with the argument value.
 * G means "Get" reply by setting through a pointer.
 * Q means "Query" response is on the return value.
 * X means "eXchange" switch G and S automatically.
 * H means "sHift" switch T and Q automatically.
 */
#define SCULL_IOCSQUANTUM		_IOW(SCULL_IOC_MAGIC,   1, int)
#define SCULL_IOCSQSET			_IOW(SCULL_IOC_MAGIC,   2, int)
#define SCULL_IOCTQUANTUM		_IO(SCULL_IOC_MAGIC,    3)
#define SCULL_IOCTQSET			_IO(SCULL_IOC_MAGIC,    4)
#define SCULL_IOCGQUANTUM		_IOR(SCULL_IOC_MAGIC,   5, int)
#define SCULL_IOCGQSET			_IOR(SCULL_IOC_MAGIC,   6, int)
#define SCULL_IOCQQUANTUM		_IO(SCULL_IOC_MAGIC,    7)
#define SCULL_IOCQQSET			_IO(SCULL_IOC_MAGIC,    8)
#define SCULL_IOCXQUANTUM		_IOWR(SCULL_IOC_MAGIC,  9, int)
#define SCULL_IOCXQSET			_IOWR(SCULL_IOC_MAGIC, 10, int)
#define SCULL_IOCHQUANTUM		_IO(SCULL_IOC_MAGIC,   11)
#define SCULL_IOCHQSET			_IO(SCULL_IOC_MAGIC,   12)

#define SCULL_IOC_MAXNR			13

#endif /* _KERNEL_IN_ACTION_SCULL_SCULL_H */
