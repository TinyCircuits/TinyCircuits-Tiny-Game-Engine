#include "linked_list.h"
#include "py/obj.h"

void linked_list_init(linked_list *list) {
    list->start = list->end = NULL;
    list->count = 0;
    list->initialized = false;
}


// Internal function for creating a new 'linked_list_node'
linked_list_node *setup_new_node(linked_list *list){
    // Allocate a new node, set defaults
    linked_list_node *new_node = list->end->next;
    new_node = m_tracked_calloc(1, sizeof(linked_list_node));
    new_node->next = NULL;
    new_node->previous = NULL;
    new_node->object = NULL;

    // Increase the count of elemets in this linked list
    list->count++;

    return new_node;
}


// Returns pointer to the node in the list so that it can be removed easily. Adds node to end
linked_list_node *linked_list_add_obj(linked_list *list, void *obj){
    ENGINE_INFO_PRINTF("Linked List: adding object");

    // Allocate a new node to hold the new object, set defaults
    linked_list_node *new_node = setup_new_node(list);
    new_node->object = obj;

    // Set the start node to the new node if not initialized and point the end to it
    if(list->initialized == false){
        ENGINE_INFO_PRINTF("Linked List: initializing list");
        new_node->previous = NULL;
        new_node->next = NULL;
        list->start = new_node;
        list->end = new_node;
        list->count = 1;    // One object added when list initialized
        list->initialized = true;
    }else{
        new_node->previous = list->end;
        new_node->next = NULL;
        list->end->next = new_node;
        list->end = new_node;
    }

    // Return pointer to the new node so it can be removed easily
    return new_node;
}


// Remove a node from the list and free it
void linked_list_del_list_node(linked_list *list, linked_list_node *node){
    ENGINE_INFO_PRINTF("Linked List: removing object");

    // Only the 'start' node can have a NULL previous node, relink
    // 'start' to its next if that's also not null. Otherwise, if
    // 'previous' and 'next' are NULL set list to uninitialized
    if(node != NULL){
        if(node->previous == NULL && node->next == NULL){   // Only one node in list, the 'start' node
            list->initialized = false;

            // Important to set these to null. For example, when looping
            // through the list it might be common to check if 'start' is NULL
            // before continuing on. If the object attached to the linked list
            // node were NULL but the linked list node is not then seg fault could
            // occur
            list->start = NULL;
            list->end = NULL;
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

        m_tracked_free(node);
    }

    // Decrease the count of elemets in this linked list
    list->count--;
}


void linked_list_clear(linked_list *list){
    ENGINE_INFO_PRINTF("Linked List: removing all objects...");

    // Keep deleting start node until it is made null
    while(list->start != NULL){
        linked_list_del_list_node(list, list->start);
    }
}