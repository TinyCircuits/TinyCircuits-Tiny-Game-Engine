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


// Internal function for creating a new 'linked_list_node'
linked_list_node *setup_new_node(linked_list *list){
    // Allocate a new node, set defaults
    linked_list_node *new_node = list->end->next;
    new_node = malloc(sizeof(linked_list_node));
    new_node->next = NULL;
    new_node->previous = NULL;
    new_node->object = NULL;

    return new_node;
}


// Returns pointer to the node in the list so that it can be removed easily. Adds node to end
linked_list_node *linked_list_add_obj(linked_list *list, void *obj){
    ENGINE_INFO_PRINTF("Object List: adding object");

    // Allocate a new node to hold the new object, set defaults
    linked_list_node *new_node = setup_new_node(list);

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


/*  Returns pointer to the node in the list so that it can be removed easily. Uses the
    passed sort function to add the object to the list. For example, if the 'object' attribute
    of our linked list nodes contains a struct as follows:

    typedef struct{
        uint8_t height;
    } person;

    We could write a function to add 'person' 'obj's to the linked list as follows:

    bool person_sort(person *new_person, person *existing_person){
        if(new_person->height > existing_person->height){
            return true;
        }

        return false;
    }

    Once 'new_person' is taller than 'existing_person', 'new_person' will be added to list after
    'existing_person'. Therefore, this function determines when the new object will be added after
    an element. Internally, if we compare against all persons and find that we were shorter than
    all of them, the new person would be added to the start of the list
*/
linked_list_node *linked_list_sorted_add_obj(linked_list *list, void *obj, bool (*compare_sort_func)()){
    ENGINE_INFO_PRINTF("Object List: adding object, sorted");

    // If not initialzed yet there won't be anything 
    // to sort, use the normal add object function
    if(list->initialzed == false){
        return linked_list_add_obj(list, obj);
    }

    // List is initialzed, allocate a new node to hold the new object, set defaults
    linked_list_node *new_node = setup_new_node(list);
    new_node->object = obj;

    // To begin sorting we need a node to start with
    linked_list_node *current_node = list->start;

    while(current_node != NULL){

        if(compare_sort_func(obj, current_node->object)){
            // Once its found that the new object has an attribute that is
            // larger than existing one it will be added after the existing
            // node. If the existing node is an end node, then the new node
            // will be the new end node. Otherwise, it gets placed between
            // two other nodes
            if(current_node->next == NULL){ // Is the current node an end node?
                new_node->previous = current_node;  // The new end node should point to the previous end node through 'previous'
                new_node->next = NULL;              // End nodes do not have a next node
                current_node->next = new_node;      // The end node that's being replaced can have a next node that now points to the new end node
                list->end = new_node;               // Actually replace the end node with the new node
            }else{                          // Not an end node, place between two other nodes
                linked_list_node *to_be_before_new_node = current_node;
                linked_list_node *to_be_after_new_node = current_node->next;

                // Put the new node in the list
                to_be_before_new_node->next = new_node;
                to_be_after_new_node->previous = new_node;
                new_node->previous = to_be_before_new_node;
                new_node->next = to_be_after_new_node;
            }

            // Return the newly added node so it can be deleted easier in the future
            return new_node;
        }

        // Keeping getting the next node until sorted or reach list end
        current_node = current_node->next;
    }

    // Looks like the while loop never returned and therefore never
    // found a true comparision, put the node at the start
    new_node->previous = NULL;          // Start nodes never have a previous node
    new_node->next = list->start;       // This new node is the new start, make its next node the start that's being replaced/moved
    list->start->previous = new_node;   // The start node that's being replaced should point to the new start node through previous (remember this isn't the start node anymore, it can have a previous now)
    list->start = new_node;             // Actually replace the start node in the list now

    // Return pointer to the new node so it can be removed easily
    return new_node;
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