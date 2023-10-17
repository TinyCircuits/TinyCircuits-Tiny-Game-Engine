#ifndef OBJECT_LAYERS_H
#define OBJECT_LAYERS_H

#include "linked_list.h"

typedef struct{
    struct linked_list layer_list;
    uint16_t layer_index;
} object_layer;


// A dynamic linked list of 'object_layer' instances
linked_list object_layers;


void add_object_to_layer(void* obj, uint16_t layer_index){
    
}


#endif  // OBJECT_LAYERS_H