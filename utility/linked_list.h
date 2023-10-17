#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdlib.h>
#include "py/obj.h"

#include "utility/debug_print.h"


typedef struct linked_list_node{
    void *object;
    struct linked_list_node *next;
    struct linked_list_node *previous;
    bool is_end_node;
} linked_list_node;


typedef struct{
    struct linked_list_node *start;
    struct linked_list_node *end;
    bool initialzed;
} linked_list;


// Returns pointer to the node in the list so that it can be removed easily
linked_list_node *linked_list_add_obj(linked_list *list, void *obj){
    ENGINE_INFO_PRINTF("Object List: adding object");

    // Allocate a new node
    linked_list_node *new_node = list->end->next;
    new_node = malloc(sizeof(linked_list_node));

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

    // Relink nodes around the node being deleted
    if(node->previous != NULL) node->previous->next = node->next;
    if(node->next != NULL) node->next->previous = node->previous;

    // If the node being deleted is the end node,
    // set the previous node as the end node
    if(node->is_end_node && node->previous != NULL){
        node->previous->is_end_node = true;
        list->end = node->previous;
    }

    // Free the memory taken up by the now unreferenced node
    free(node);
}


#endif  // LINKED_LIST_H