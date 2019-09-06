#include <stdlib.h>
#include <string.h>

struct linkedlist_node_t {
    void *elem;                 /*!< pointer to list node data */
    struct linkedlist_node_t *next;              /*!< pointer to the next list node */
};

struct linkedlist_t{
	struct linkedlist_node_t *first;
	struct linkedlist_node_t *last;
	int size;
};


struct linkedlist_t *linkedlist_create();
void * linkedlist_insert_tail(struct linkedlist_t *l, void *e);
void * linkedlist_remove_head(struct linkedlist_t *l);
struct linkedlist_t* linkedlist_remove(struct linkedlist_t *l, void *e);
void linkedlist_destroy(struct linkedlist_t **l);
int linkedlist_size(struct linkedlist_t *l);