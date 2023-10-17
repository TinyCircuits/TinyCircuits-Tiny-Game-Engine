#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdlib.h>
#include "py/obj.h"

#include "utility/debug_print.h"


typedef struct linked_list_node{
    void *object;
    struct linked_list_node *next;
    struct linked_list_node *previous;
} linked_list_node;


typedef struct{
    struct linked_list_node *start;
    struct linked_list_node *end;
    bool initialzed;
} linked_list;


// Returns pointer to the node in the list so that it can be removed easily
linked_list_node *linked_list_add_obj(linked_list *list, void *obj){
    ENGINE_INFO_PRINTF("Object List: adding object");

    // Allocate a new node, set defaults
    linked_list_node *new_node = list->end->next;
    new_node = malloc(sizeof(linked_list_node));
    new_node->next = NULL;
    new_node->previous = NULL;
    new_node->object = NULL;

    // Set the start node to the new node if not initialzed and point the end to it
    if(list->initialzed == false){
        ENGINE_INFO_PRINTF("Object List: initializing list");
        list->start = new_node;
        list->end = list->start;
        list->initialzed = true;
    }

    // Put data in the new node and link to previous 
    new_node->object = obj;
    new_node->previous = list->end;

    // Add new node to list and set the end of the list to it
    list->end->next = new_node;
    list->end = list->end->next;

    // Return pointer to the new node so it can be removed easily
    return list->end;
}


// Remove a node from the list and free it
void linked_list_del_list_node(linked_list *list, linked_list_node *node){
    ENGINE_INFO_PRINTF("Object List: removing object");

    // Only the 'start' node can have a NULL previous node, relink 
    // 'start' to its next if that's also not null. Otherwise, if
    // 'previous' and 'next' are NULL set list to uninitialized
    if(node->previous == NULL && node->next == NULL){   // Only one node in list, the 'start' node
        list->initialzed = false;
    }else if(node->previous == NULL){                   // Deleting 'start' but there are other nodes, link start to next
        list->start = node->next;
        list->start->previous = NULL;
    }else if(node->next == NULL){                       // Deleting 'end' but there are other nodes, link end to previous
        list->end = node->previous;
        list->end->next = NULL;
    }else{                                              // Deleting a node in middle, relink around
        node->previous->next = node->next;
        node->next->previous = node->previous;
    }

    free(node);
}


#endif  // LINKED_LIST_H