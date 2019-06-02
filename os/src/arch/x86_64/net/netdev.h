#ifndef NETDEV
#define NETDEV

#include <stddef.h>
#include "net/ethernet/ethernet.h"
#include "net/ip/ipv4.h"
#include "net/if.h"
#include "types/llist.h"

typedef struct net_device_node net_device_node;
typedef struct net_device_list net_device_list;

typedef struct netdev_hw_addr_list netdev_hw_addr_list;
typedef struct netdev_hw_addr_node netdev_hw_addr_node;

typedef struct netdev_ip_addr_list netdev_ip_addr_list;
typedef struct netdev_ip_addr_node netdev_ip_addr_node;

typedef struct net_device net_device;


/* structs for storing list of net devices */
struct net_device_node {
	llist_node node;
	net_device *dev;
};

struct net_device_list {
	llist_head list;
	int count;
};

/* structs for storing list of net device's hw addresses */
struct netdev_hw_addr_node {
	llist_node node;
	hw_addr addr;
};

struct netdev_hw_addr_list {
        llist_head list;
	int count;
};

/* structs for storing list of net device's ip addresses */
struct netdev_ip_addr_node {
	llist_node node;
	ipv4_addr addr;
};

struct netdev_ip_addr_list {
	llist_head list;
	int count;
};

struct net_device {
	char name[IFNAMSIZ];

	void *netdev_priv;
	
        netdev_hw_addr_list dev_addrs;
	netdev_ip_addr_list ipv4_addrs;
};

net_device *alloc_netdev(char *name, size_t priv_size);

int add_net_dev(net_device_list devs, net_device *new_dev);

int add_dev_addr(net_device *dev, hw_addr addr_to_add);
void print_dev_addrs(net_device *dev);

int add_ipv4_addr(net_device *dev, ipv4_addr addr_to_add);

#endif
