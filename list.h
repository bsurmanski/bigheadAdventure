/**
 *  list.h
 *  platform-fun
 *  October 04, 2011
 *  Brandon Surmanski
 */

#ifndef _LIST_H
#define _LIST_H

typedef struct List List;

typedef struct Node Node;

List *list_create(size_t element_size);
void list_delete(List *list);

void list_prepend(List *list, void *element);
void list_append(List *list, void *element);
void list_remove(List *list, Node *element);
void list_sort(List *list, int (*sort)(void *a, void *b));
size_t list_length(List *list);

Node *list_first_node(List *list);
Node *list_last_node(List *list);
Node *node_next(Node *n);
Node *node_previous(Node *n);
void *node_value(Node *n);


#endif
