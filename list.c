/**
 * list.c
 * platform-fun
 * October 04, 2011
 * Brandon Surmanski
 */

#include <stdlib.h>
#include <string.h>

#include "list.h"

struct Node {
    Node *prev;
    Node *next;
    void *value;
};

struct List {
    size_t length;
    size_t element_size;
    Node* first;
    Node* last;
};

/**
 * creates a list of length 0 and returns a ptr 
 */
List *list_create(size_t element_size)
{
    List *l = malloc (sizeof(List));
    l->length = 0;
    l->element_size = element_size;
    l->first = 0;
    l->last = 0;
    return l;
}

/**
 * deletes the list and frees all entries
 */
void list_delete(List *list)
{
    Node *n = list->first;
    while(n){
        Node *next = n->next;
        free(n->value);
        free(n);
        n = next;
    }
    free(list);
}


/**
 * prepends a node to the beginning of the list.
 */
void list_prepend(List *list, void *element)
{
    Node *new_node = malloc(sizeof(Node));
    Node *first = list->first;

    new_node->prev = 0;
    new_node->next = first;
    new_node->value = malloc(list->element_size);
    memcpy(new_node->value, element, list->element_size);

    if(first){
        first->prev = new_node;
    } else {
        list->last = new_node;
    }

    list->first = new_node;

    ++(list->length);
}


/**
 * appends a node to the end of the list
 */
void list_append(List *list, void *element)
{
    Node *new_node = malloc(sizeof(Node));
    if (new_node){
        Node *last = list->last;

        new_node->prev = last;
        new_node->next = 0;
        new_node->value = malloc(list->element_size);
        memcpy(new_node->value, element, list->element_size);

        if(last){
            last->next = new_node;
        } else {
            list->first = new_node;
        }

        list->last = new_node;

        ++(list->length);
    }
}

/**
 * remove the node give by *node. it is expected that *node is resident in 
 * *list. 
 */
void list_remove(List *list, Node *node)
{
    Node *before = node->prev;
    Node *after = node->next;
    free(node->value);
    free(node);

    if(before){
        before->next = after;    
    } else {
        list->first = after;
    }

    if(after){
        after->prev = before;
    } else {
        list->last = before;
    }

    --(list->length);
}

void list_sort(List *list, int (*sort)(void *a, void *b))
{
    // UNIMPLIMENTED
}

size_t list_length(List *list)
{
    return list->length;
}

Node *list_first_node(List *list)
{
    return list->first;
}

Node *list_last_node(List *list)
{
    return list->last;
}

Node *node_next(Node *n)
{
    return n->next;
}

Node *node_previous(Node *n)
{
    return n->prev;
}

void *node_value(Node *n)
{
    return n->value;
}
