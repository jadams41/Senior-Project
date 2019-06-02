#include <stddef.h>
#include "types/llist.h"
#include "net/netdev.h"
#include "drivers/memory/memoryManager.h"
#include "utils/printk.h"
#include "types/string.h"

net_device *alloc_netdev(char *name, size_t priv_size){
	net_device *new_dev;

	/* allocate new net_dev structure */
	new_dev = (net_device*)kmalloc(sizeof(net_device));

	/* allocate new netdev_priv structure */
	new_dev->netdev_priv = kmalloc(priv_size);

	/* set device's name */
	strcpy(new_dev->name, name);

	/* initialize hw address list */
        new_dev->dev_addrs.count = 0;
	new_dev->dev_addrs.list.first = NULL;
	new_dev->dev_addrs.list.last = NULL;
	
	/* initialize ipv4 address list */
        new_dev->ipv4_addrs.count = 0;
	new_dev->ipv4_addrs.list.first = NULL;
	new_dev->ipv4_addrs.list.last = NULL;

	return new_dev;
}

int add_net_dev(net_device_list devs, net_device *new_dev){
        net_device_node *n = (net_device_node*)kmalloc(sizeof(net_device_node));
	n->node.next = NULL;
	n->node.prev = NULL;
	n->dev = new_dev;
	
	if(devs.list.first == NULL){
	        devs.list.last = devs.list.first = (llist_node*)n;
		return devs.count = 1;
	}

        net_device_node *last = (net_device_node*)(devs.list.last);
	n->node.prev = (llist_node*)(&(last->node));
	last->node.next = (llist_node*)n;
        devs.list.last = (llist_node*)n;

	devs.count += 1;

	return devs.count;
}

int add_dev_addr(net_device *dev, hw_addr addr_to_add){
	netdev_hw_addr_node *n = (netdev_hw_addr_node*)kmalloc(sizeof(netdev_hw_addr_node));
	n->node.next = NULL;
	n->node.prev = NULL;
	n->addr = addr_to_add;
	
	if(dev->dev_addrs.list.first == NULL){
		dev->dev_addrs.list.last = dev->dev_addrs.list.first = (llist_node*)n;
		return dev->dev_addrs.count = 1;
	}

        netdev_hw_addr_node *last = (netdev_hw_addr_node*)(dev->dev_addrs.list.last);
	n->node.prev = (llist_node*)(&(last->node));
	last->node.next = (llist_node*)n;
	dev->dev_addrs.list.last = (llist_node*)n;

	dev->dev_addrs.count += 1;

	return dev->dev_addrs.count;
}

void print_dev_addrs(net_device *dev){
	printk("NetDev %s has %d hw addresses:\n", dev->name, dev->dev_addrs.count);
	netdev_hw_addr_node *cur = (netdev_hw_addr_node*)&(dev->dev_addrs.list.first);
	int i = 0;
        while(cur != NULL){
		char *cur_hw_addr_str = hw_addr_to_str(cur->addr);
		printk("%d) %s\n", ++i, cur_hw_addr_str);
		cur = (netdev_hw_addr_node*)(cur->node.next);
	}
}

/* int remove_dev_addr(hw_addr addr_to_remove){ */
	
/* } */

int add_ipv4_addr(net_device *dev, ipv4_addr addr_to_add){
	netdev_ip_addr_node *n = (netdev_ip_addr_node*)kmalloc(sizeof(netdev_ip_addr_node));
	n->node.next = NULL;
	n->node.prev = NULL;
	n->addr = addr_to_add;
	
	if(dev->ipv4_addrs.list.first == NULL){
		dev->ipv4_addrs.list.last = dev->ipv4_addrs.list.first = (llist_node*)n;
		return dev->ipv4_addrs.count = 1;
	}

        netdev_ip_addr_node *last = (netdev_ip_addr_node*)(dev->dev_addrs.list.last);
	n->node.prev = (llist_node*)(&(last->node));
	last->node.next = (llist_node*)n;
	dev->ipv4_addrs.list.last = (llist_node*)n;

	return ++dev->ipv4_addrs.count;
}

