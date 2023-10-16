#ifndef OBJECT_LIST_H
#define OBJECT_LIST_H

#include <stdlib.h>
#include "py/obj.h"

#include "utility/debug_print.h"


typedef struct object_list_node{
    void *object;
    struct object_list_node *next;
    struct object_list_node *previous;
    bool is_end_node;
} object_list_node;


typedef struct{
    struct object_list_node root;
    struct object_list_node *end;
    bool initialzed;
} object_list;


// Initialize a 'active_objects_list' data stucture
void init_object_list(object_list *list){
    ENGINE_INFO_PRINTF("Object List: list initialzed");

    // Make the root node the end node, set it as end, and set struct instance as initialzed
    list->root.next = &list->root;
    list->root.previous = NULL;
    list->root.object = NULL;

    list->end = &list->root;
    list->end->is_end_node = true;

    list->initialzed = true;
}


// Returns pointer to the node in the list so that it can be removed easily
object_list_node *object_list_add_obj(object_list *list, void *obj){
    ENGINE_INFO_PRINTF("Object List: adding object");

    // If the list has not been initialzed yet, do it
    if(list->initialzed == false){
        init_object_list(list);
    }

    // Allocate another node after the current end node
    // Set the next of the new node as NULL (nothing after newest) and
    // set the previous node of this new node to the last end
    list->end->next = malloc(sizeof(object_list_node));
    list->end->next->object = obj;
    list->end->next->next = NULL;
    list->end->next->previous = list->end;
    list->end->next->is_end_node = true;

    // Replace the last end node with the newest
    list->end->is_end_node = false;
    list->end = list->end->next;

    // Return pointer to newest end node
    return list->end;
}


void object_list_del_list_node(object_list *list, object_list_node *node){
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


#endif  // OBJECT_LIST_H