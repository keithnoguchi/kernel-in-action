/* SPDX-License-Identifier: GPL-2.0 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ip.h>

/* device lockup parameters */
static int tx_lockup = 0;
module_param(tx_lockup, int, S_IRUGO|S_IWUSR);

/* sn0 and sn1 */
static struct net_device *netdevs[2];

static struct net_device *dest_dev(const struct net_device *dev)
{
	return dev == netdevs[0] ? netdevs[1] : netdevs[0];
}

/* snull packet buffer */
struct snull_buff {
	struct snull_buff	*next;
	struct net_device	*dev;
	int			datalen;
	u8			data[ETH_DATA_LEN];
};

/* snull device */
struct snull_dev {
	spinlock_t		lock;
	int			status;
#define SNULL_TX_INTR		(1 << 0)
#define SNULL_RX_INTR		(1 << 1)
	struct sk_buff		*skb;		/* inflight TX skb */
	int			datalen;	/* inflight TX data len */
	struct snull_buff	*pool;
	struct snull_buff	*rx_queue;
	irqreturn_t (*interrupt)(int, void *, struct pt_regs *);
};

static inline int snull_tx_lockup(void)
{
	int val;

	/* lock is required, as in linux/moduleparam.h */
	kernel_param_lock(THIS_MODULE);
	val = tx_lockup;
	kernel_param_unlock(THIS_MODULE);

	return val;
}

static int init_pool(struct net_device *dev)
{
	struct snull_dev *s = netdev_priv(dev);
	struct snull_buff *b;
	int i;

	s->pool = NULL;
	for (i = 0; i < 8; i++) {
		b = kzalloc(sizeof(struct snull_buff), GFP_KERNEL);
		if (unlikely(!b))
			return -ENOMEM;
		b->next = s->pool;
		s->pool = b;
	}
	return 0;
}

static void free_pool(struct net_device *dev)
{
	struct snull_dev *s = netdev_priv(dev);
	struct snull_buff *b;

	while ((b = s->pool)) {
		s->pool = b->next;
		kfree(b);
	}
}

static struct snull_buff *dequeue_pool(struct net_device *dev)
{
	struct snull_dev *s = netdev_priv(dev);
	struct snull_buff *b = s->pool;
	unsigned long flags;

	spin_lock_irqsave(&s->lock, flags);
	b = s->pool;
	s->pool = b->next;
	if (!s->pool)
		netif_stop_queue(dev); /* no more buffer in the pool */
	spin_unlock_irqrestore(&s->lock, flags);

	return b;
}

void enqueue_pool(struct net_device *dev, struct snull_buff *b)
{
	struct snull_dev *s = netdev_priv(dev);
	unsigned long flags;

	spin_lock_irqsave(&s->lock, flags);
	b->next = s->pool;
	s->pool = b;
	if (netif_queue_stopped(dev) && !b->next)
		netif_wake_queue(dev);
	spin_unlock_irqrestore(&s->lock, flags);
}

static void free_rx(struct net_device *dev)
{
	struct snull_dev *s = netdev_priv(dev);
	struct snull_buff *b;

	while ((b = s->rx_queue)) {
		s->rx_queue = b->next;
		kfree(b);
	}
}

struct snull_buff *dequeue_rx(struct net_device *dev)
{
	struct snull_dev *s = netdev_priv(dev);
	struct snull_buff *pkt;
	unsigned long flags;

	spin_lock_irqsave(&s->lock, flags);
	pkt = s->rx_queue;
	if (pkt)
		s->rx_queue = pkt->next;
	spin_unlock_irqrestore(&s->lock, flags);

	return pkt;
}

static void enqueue_rx(struct net_device *dev, struct snull_buff *pkt)
{
	struct snull_dev *s = netdev_priv(dev);
	struct snull_buff **bp;
	unsigned long flags;

	pkt->next = NULL;
	spin_lock_irqsave(&s->lock, flags);
	for (bp = &s->rx_queue; *bp; bp = &(*bp)->next)
		; /* skip to the tail */
	*bp = pkt;
	spin_unlock_irqrestore(&s->lock, flags);
}

static void snull_interrupt(struct net_device *dev, int interrupt)
{
	struct snull_dev *s = netdev_priv(dev);
	unsigned long flags;

	spin_lock_irqsave(&s->lock, flags);
	s->status |= interrupt;
	spin_unlock_irqrestore(&s->lock, flags);

	if (s->interrupt)
		s->interrupt(0, dev, NULL);
}

static int snull_hw_tx(char *data, int len, struct net_device *dev)
{
	struct net_device *dst;
	struct snull_buff *b;
	struct iphdr *ih;
	u32 *saddr, *daddr;
	int lockup;

	netdev_info(dev, "%s\n", __FUNCTION__);

	/* to avoid the crash */
	if (len < (sizeof(struct ethhdr) + sizeof(struct iphdr))) {
		netdev_warn(dev, "ignore short packet(len=%d)\n", len);
		return -EINVAL;
	}

	/* flip the least significant bit of the third octects */
	ih = (struct iphdr *)(data + sizeof(struct ethhdr));
	saddr = &ih->saddr;
	daddr = &ih->daddr;
	((u8 *)saddr)[2] ^= 1;
	((u8 *)daddr)[2] ^= 1;

	/* recalculate the IP header checksum */
	ih->check = 0;
	ih->check = ip_fast_csum((const void *)ih, ih->ihl);

	/* get the buffer to hold the new packet */
	b = dequeue_pool(dev);
	if (unlikely(!b))
		return -ENOMEM;
	b->datalen = len;
	memcpy(b->data, data, len);

	/* put it in the destination RX queue, and trigger the interrupt */
	dst = dest_dev(dev);
	enqueue_rx(dst, b);
	snull_interrupt(dst, SNULL_RX_INTR);

	/* simulate the TX lockup */
	lockup = snull_tx_lockup();
	if (unlikely(lockup && ((dev->stats.tx_packets + 1) % lockup) == 0)) {
		netif_stop_queue(dev);
	} else {
		/* done with the tx */
		snull_interrupt(dev, SNULL_TX_INTR);
	}

	return 0;
}

/* net_device_ops */
static int snull_open(struct net_device *dev)
{
	netdev_info(dev, "%s\n", __FUNCTION__);

	/* initialize the TX buffers. */
	init_pool(dev);

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
	netdev_info(dev, "%s\n", __FUNCTION__);
	netif_stop_queue(dev);
	free_pool(dev);
	free_rx(dev);
	return 0;
}

static netdev_tx_t snull_tx(struct sk_buff *skb, struct net_device *dev)
{
	struct snull_dev *s = netdev_priv(dev);
	char *data, shortpkt[ETH_ZLEN];
	unsigned long flags;
	int len;

	netdev_info(dev, "%s\n", __FUNCTION__);

	data = skb->data;
	len = skb->len;
	if (len < ETH_ZLEN) {
		memset(shortpkt, 0, ETH_ZLEN);
		memcpy(shortpkt, skb->data, skb->len);
		data = shortpkt;
		len = ETH_ZLEN;
	}

	/* XXX inflight skb, to be freed at IRQ handler */
	spin_lock_irqsave(&s->lock, flags);
	s->datalen = len;
	s->skb = skb;
	spin_unlock_irqrestore(&s->lock, flags);

	if (snull_hw_tx(data, len, dev)) {
		dev->stats.tx_errors++;
		return NETDEV_TX_BUSY;
	}
	return NETDEV_TX_OK;
}

static void snull_tx_timeout(struct net_device *dev)
{
	netdev_info(dev, "%s\n", __FUNCTION__);
}

const static struct net_device_ops snull_ops = {
	.ndo_open	= snull_open,
	.ndo_stop	= snull_release,
	.ndo_start_xmit	= snull_tx,
	.ndo_tx_timeout	= snull_tx_timeout,
};

/* header_ops */
static int snull_header(struct sk_buff *skb, struct net_device *dev,
			unsigned short type, const void *daddr,
			const void *saddr, unsigned int len)
{
	struct ethhdr *eth = skb_push(skb, ETH_HLEN);

	if (type != ETH_P_802_3 && type != ETH_P_802_2)
		eth->h_proto = htons(type);
	else
		eth->h_proto = htons(len);

	if (!saddr)
		saddr = dev->dev_addr;
	memcpy(eth->h_source, saddr, ETH_ALEN);

	if (!daddr)
		daddr = dev->dev_addr;
	memcpy(eth->h_dest, daddr, ETH_ALEN);

	eth->h_dest[ETH_ALEN-1]	^= 0x01; /* dest is us xor 1 */

	return ETH_HLEN;
}

const static struct header_ops snull_header_ops = {
	.create		= snull_header,
};

static int snull_rx(struct net_device *dev, struct snull_buff *pkt)
{
	struct sk_buff *skb;
	int err = -ENOMEM;

	netdev_info(dev, "%s\n", __FUNCTION__);

	skb = dev_alloc_skb(pkt->datalen + 2);
	if (unlikely(!skb)) {
		if (printk_ratelimit())
			netdev_warn(dev, "low on mem - dropped");
		dev->stats.rx_dropped++;
		goto out;
	}
	skb_reserve(skb, 2); /* 16 alignment */
	memcpy(skb_put(skb, pkt->datalen), pkt->data, pkt->datalen);

	skb->dev = dev;
	skb->protocol = eth_type_trans(skb, dev);
	skb->ip_summed = CHECKSUM_UNNECESSARY;
	dev->stats.rx_packets++;
	dev->stats.rx_bytes += pkt->datalen;
	err = netif_rx(skb);
out:
	return err;
}

/* old/regular interrupt handler */
static irqreturn_t snull_regular_interrupt(int irq, void *dev_id,
					   struct pt_regs *regs)
{
	struct net_device *dev = (struct net_device *) dev_id;
	struct snull_dev *s;
	int status;
	int err = 0;

	if (!dev)
		return IRQ_NONE;

	/* status flag */
	s = netdev_priv(dev);
	spin_lock(&s->lock);
	status = s->status;
	s->status = 0;
	spin_unlock(&s->lock);

	if (status & SNULL_RX_INTR) {
		struct snull_buff *pkt = dequeue_rx(dev);
		if (pkt) {
			err = snull_rx(dev, pkt);
			/* queue it back to the source dev pool */
			enqueue_pool(dest_dev(dev), pkt);
		}
	}
	if (status & SNULL_TX_INTR) {
		struct sk_buff *skb;
		int datalen;

		spin_lock(&s->lock);
		datalen = s->datalen;
		s->datalen = 0;
		skb = s->skb;
		s->skb = NULL;
		spin_unlock(&s->lock);
		if (skb)
			dev_kfree_skb(skb);
		if (datalen) {
			dev->stats.tx_packets++;
			dev->stats.tx_bytes += datalen;
		}
	}

	if (err)
		return IRQ_NONE;
	return IRQ_HANDLED;
}

static void snull_init(struct net_device *dev)
{
	struct snull_dev *s = netdev_priv(dev);

	netdev_info(dev, "%s\n", __FUNCTION__);
	ether_setup(dev);
	dev->netdev_ops	= &snull_ops;
	dev->header_ops = &snull_header_ops;
	dev->flags	|= IFF_NOARP;
	s->interrupt	= snull_regular_interrupt;

	spin_lock_init(&s->lock);
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
