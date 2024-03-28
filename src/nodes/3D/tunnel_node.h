#ifndef TUNNEL_NODE_H
#define TUNNEL_NODE_H

#include "py/obj.h"
#include "nodes/node_base.h"

// https://lodev.org/cgtutor/tunnel.html
typedef struct{
    mp_obj_t texture_resource;      // TextureResource
}engine_tunnel_node_class_obj_t;

extern const mp_obj_type_t engine_tunnel_node_class_type;
void tunnel_node_class_draw(engine_node_base_t *line_node_base, mp_obj_t camera_node);

#endif  // TUNNEL_NODE_H