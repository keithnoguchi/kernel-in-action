/* SPDX-License-Identifier: GPL-2.0 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>

/* sn0 and sn1 */
static struct net_device *netdevs[2];

/* snull driver */
struct snull_dev { };

/* net_device_ops */
static int snull_open(struct net_device *dev)
{
	pr_info("%s(%s)\n", __FUNCTION__, netdev_name(dev));

	/*
	 * Assign the hardware address of the board: use "\0SNULx",
	 * where x is 0 or 1.  The first byte is '\0' to avoid being
	 * a multicast address (the first byte of multicat addrs is odd).
	 */
	memcpy(dev->dev_addr, "\0SNUL0", ETH_ALEN);
	if (dev == netdevs[1])
		dev->dev_addr[ETH_ALEN-1]++;
	netif_start_queue(dev);
	return 0;
}

static int snull_release(struct net_device *dev)
{
	pr_info("%s(%s)\n", __FUNCTION__, netdev_name(dev));
	netif_stop_queue(dev);
	return 0;
}

static netdev_tx_t snull_tx(struct sk_buff *skb, struct net_device *dev)
{
	char *data, shortpkt[ETH_ZLEN];
	int len;

	pr_info("%s(%s)\n", __FUNCTION__, netdev_name(dev));

	data = skb->data;
	len = skb->len;
	if (len < ETH_ZLEN) {
		memset(shortpkt, 0, ETH_ZLEN);
		memcpy(shortpkt, skb->data, skb->len);
		data = shortpkt;
		len = ETH_ZLEN;
	}
	return NETDEV_TX_OK;
}

static void snull_tx_timeout(struct net_device *dev)
{
	pr_info("%s(%s)\n", __FUNCTION__, netdev_name(dev));
}

const static struct net_device_ops snull_ops = {
	.ndo_open	= snull_open,
	.ndo_stop	= snull_release,
	.ndo_start_xmit	= snull_tx,
	.ndo_tx_timeout	= snull_tx_timeout,
};

static void snull_init(struct net_device *dev)
{
	pr_info("%s(%s)\n", __FUNCTION__, netdev_name(dev));
	ether_setup(dev);
	dev->netdev_ops	= &snull_ops;
	dev->flags	|= IFF_NOARP;
}

static int __init snull_init_module(void)
{
	int i;

	pr_info("%s\n", __FUNCTION__);

	for (i = 0; i < ARRAY_SIZE(netdevs); i++) {
		struct net_device *dev;
		int err;

		dev = alloc_netdev(sizeof(struct snull_dev), "sn%d",
				   NET_NAME_UNKNOWN, snull_init);
		if (!dev)
			goto unregister;

		err = register_netdev(dev);
		if (err) {
			free_netdev(dev);
			goto unregister;
		}
		netdevs[i] = dev;
	}
	return 0;
unregister:
	for (i = 0; i < ARRAY_SIZE(netdevs); i++) {
		if (!netdevs[i])
			continue;
		unregister_netdev(netdevs[i]);
		free_netdev(netdevs[i]);
		netdevs[i] = NULL;
	}
	return -ENOMEM;
}
module_init(snull_init_module);

static void __exit snull_cleanup_module(void)
{
	int i;

	pr_info("%s\n", __FUNCTION__);
	for (i = 0; i < ARRAY_SIZE(netdevs); i++) {
		unregister_netdev(netdevs[i]);
		free_netdev(netdevs[i]);
	}
}
module_exit(snull_cleanup_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kei Nohguchi <kei@nohguchi.com>");
MODULE_DESCRIPTION("LDD's Simple Network device");
