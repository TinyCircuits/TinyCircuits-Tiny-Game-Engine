#ifndef OBJECT_POINTER_SLAB_ALLOCATOR_H
#define OBJECT_POINTER_SLAB_ALLOCATOR_H

#include "py/obj.h"


typedef struct{
    mp_obj_t *active_objects;                   // Dynamicly allocated array of pointers to mp_obj_ts
    uint16_t active_objects_size = 0;           // The number of slabs in 'active_objects' (i.e. max number of slots ever possible in 'active objects')
    uint16_t active_object_count = 0;           // The number of active objects (i.e. slots used in 'active_objects')   

    uint16_t *unused_active_object_indices;     // Dynamicly allocated array of indices into 'active_objects'
    uint16_t unused_active_object_count = 0;    // The number of unused active object slabs left
} obj_ptr_slab_allocator;


void init_obj_ptr_slab_allocator(obj_ptr_slab_allocator *slab, uint16_t number_of_objects){
    slab->active_objects = malloc(sizeof(object_pointer_slab_allocator) * number_of_objects);
    slab->active_objects_size = number_of_objects;
    slab->active_object_count = 0;

    slab->unused_active_object_indices = malloc(sizeof(uint16_t) * number_of_objects);
    slab->unused_active_object_count = number_of_objects;

    // Map all unused indices into 'active_objects' (since we just inited then no slots are used)
    for(uint16_t iobjx=0; iobjx<slab->unused_active_object_count; iobjx++){
        slab->unused_active_object_indices[iobjx] = iobjx;
    }
}


void deinit_obj_ptr_slab_allocator(obj_ptr_slab_allocator *slab){
    free(slab->active_objects);
    free(slab->unused_active_object_indices);
}


uint16_t obj_ptr_slab_allocator_add_obj(obj_ptr_slab_allocator *slab, mp_obj_t *obj){
    // Fetch an index into an unused slot in 'active_objects' for 
    // the pointer of this object to live in. 
    int16_t unused_active_object_index = slab->unused_active_object_indices[slab->unused_active_object_count-1];
    slab->unused_active_object_count--;

    slab->active_objects[]

    slab->active_object_count++;
}


#endif  // OBJECT_POINTER_SLAB_ALLOCATOR_H