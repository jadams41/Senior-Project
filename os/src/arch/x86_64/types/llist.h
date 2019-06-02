#ifndef L_LIST_H
#define L_LIST_H

/* code for general linked list structure */
typedef struct llist_head llist_head;
typedef struct llist_node llist_node;

/* add to structure that contains a linked list */
struct llist_head {
	llist_node *first;
	llist_node *last;
};

/* add to structure that is being placed in linked list */
struct llist_node {
	llist_node *next;
	llist_node *prev;
};

#endif
