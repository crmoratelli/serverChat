/** 
	Linked list simple implementation. 
*/
#include <stdlib.h>
#include "linkedlist.h"




/**
 * @brief Create a new linked list.
 * 
 * @return Pointer to the list structure.
 */
 struct linkedlist_t *linkedlist_create(){
	struct linkedlist_t *l = (struct linkedlist_t*)malloc(sizeof(struct linkedlist_t));
	memset(l, 0, sizeof(struct linkedlist_t));
	return l;
}


/**
 * @brief Insert new element to the end of the list.
 * 
 * @param l Pointer to the list.
 * @param e Element to be inserted in the list.
 *
 * @return Pointer to the element removed.
 */
void * linkedlist_insert_tail(struct linkedlist_t *l, void *e){
	struct linkedlist_node_t *n = (struct linkedlist_node_t*)malloc(sizeof(struct linkedlist_node_t));
	n->elem = e;

	if (l->last){
		(l->last)->next = n;
	}else{
		l->first = n;
	}

	n->next = NULL;
	l->last = n;

	l->size++;

	return e;
}


/**
 * @brief Remove element from the head of the list.
 * 
 * @param l Pointer to the list.
 *
 * @return Pointer to the element removed.
 */
void * linkedlist_remove_head(struct linkedlist_t *l){
	struct linkedlist_node_t * n;
	void *e;

	if (l->first == NULL){
		return NULL;
	}

	n = l->first;
	e = n->elem;
	l->first = n->next;

	if(l->first == NULL){
		l->last = NULL;
	}

	free(n);

	l->size--;

	return e;
}

/**
 * @brief Remove an intermediate node. 
 * 
 * @param l Pointer to the list.
 * @param n Node to be removed.
 *
 * @return Pointer to the linkedlist. 
 */
struct linkedlist_t* linkedlist_remove(struct linkedlist_t *l, void *e){
	struct linkedlist_node_t *n = l->first;
	struct linkedlist_node_t *aux = NULL;

	if(n == NULL || e == NULL){
		return l;
	}

	if(n->elem == e){
		aux = l->first;
		l->first = (l->first)->next;
	}else{
		while (n->next && (n->next)->elem != e){
			n = n->next;
		}
		if(n->next){
			aux = n->next;
			n->next = (n->next)->next;
		}
	}

	if(l->first == NULL){
		l->last = NULL;
	}

	if(aux){
		free(aux);
		l->size--;
	}

	return l;
}


/**
 * @brief Remove all elements of the list and free it. 
 * 
 * @param l Pointer to the list.
 */
void linkedlist_destroy(struct linkedlist_t **l){
	struct linkedlist_node_t * n;

	while((*l)->first){
		n = (*l)->first;
		(*l)->first = n->next;
		free(n->elem);
		free(n);
	}

	free(*l);

	*l = NULL;
}


/**
 * @brief Remove all elements of the list and free it. 
 * 
 * @param l Pointer to the list.
 *
 * @return Number of elements in the list.
 */
int linkedlist_size(struct linkedlist_t *l){
	return l->size;
}