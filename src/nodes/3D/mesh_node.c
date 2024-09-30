#include "mesh_node.h"

#include "py/objstr.h"
#include "py/objtype.h"
#include "nodes/node_types.h"
#include "debug/debug_print.h"
#include "engine_object_layers.h"
#include "nodes/3D/camera_node.h"
#include "math/vector3.h"
#include "math/rectangle.h"
#include "draw/engine_display_draw.h"
#include "math/engine_math.h"
#include "draw/engine_color.h"
#include "nodes/3D/camera_node.h"
#include "display/engine_display_common.h"
#include "draw/engine_display_draw.h"
#include "draw/engine_shader.h"
#include "resources/engine_mesh_resource.h"


// Defined in camera_node.h
extern const vector3_class_obj_t world_north;


void mesh_node_set_translation(void *user_ptr){
    engine_mesh_node_class_obj_t *mesh = user_ptr;
    vector3_class_obj_t *p = mesh->position;
    glm_translate_make(mesh->m_translation, (vec3){-p->x.value, -p->y.value, -p->z.value});
}


void mesh_node_set_rotation(void *user_ptr){
    engine_mesh_node_class_obj_t *mesh = user_ptr;
    vector3_class_obj_t *r = (vector3_class_obj_t*)mesh->rotation;
    glm_euler((vec3){-r->x.value, -r->y.value, -r->z.value}, mesh->m_rotation);
}


void mesh_node_set_scale(void *user_ptr){
    engine_mesh_node_class_obj_t *mesh = user_ptr;
    vector3_class_obj_t *s = (vector3_class_obj_t*)mesh->scale;
    glm_scale_make(mesh->m_scale, (vec3){s->x.value, s->y.value, s->z.value});
}


void mesh_node_class_draw(mp_obj_t mesh_node_base_obj, mp_obj_t camera_node){
    engine_node_base_t *mesh_node_base = mesh_node_base_obj;
    engine_mesh_node_class_obj_t *mesh_node = mesh_node_base->node;
    mesh_resource_class_obj_t *mesh = mesh_node->mesh;

    // Nothing to draw if no mesh
    if(mesh == mp_const_none){
        return;
    }

    mp_obj_list_t *vertices = mesh->vertices;

    // No triangles to draw
    if(vertices->len < 3){
        return;
    }

    // mp_obj_list_t *indices = mesh->indices;
    // mp_obj_list_t *uvs = mesh->uvs;
    mp_obj_list_t *triangle_colors = mesh->triangle_colors;
    // ENGINE_PRINTF("%ld\n", triangle_colors->len);

    color_class_obj_t *mesh_color = mesh_node->color;

    engine_node_base_t *camera_node_base = camera_node;
    engine_camera_node_class_obj_t *camera = camera_node_base->node;

    engine_shader_t *shader = engine_get_builtin_shader(EMPTY_SHADER);

    mat4 m_view0 = GLM_MAT4_ZERO_INIT;
    glm_mat4_mul(camera->m_rotation, camera->m_translation, m_view0);

    mat4 m_view1 = GLM_MAT4_ZERO_INIT;
    glm_mat4_mul(mesh_node->m_rotation, mesh_node->m_translation, m_view1);

    mat4 m_view = GLM_MAT4_ZERO_INIT;
    glm_mat4_mul(m_view0, m_view1, m_view);

    mat4 mvp = GLM_MAT4_ZERO_INIT;
    glm_mat4_mul(camera->m_projection, m_view, mvp);

    for(uint16_t ivx=0; ivx<vertices->len; ivx+=3){
        vector3_class_obj_t *vertex_0 = vertices->items[ivx];
        vector3_class_obj_t *vertex_1 = vertices->items[ivx+1];
        vector3_class_obj_t *vertex_2 = vertices->items[ivx+2];

        vec3 v0 = {vertex_0->x.value, vertex_0->y.value, vertex_0->z.value};
        vec3 v1 = {vertex_1->x.value, vertex_1->y.value, vertex_1->z.value};
        vec3 v2 = {vertex_2->x.value, vertex_2->y.value, vertex_2->z.value};

        vec3 out_0 = GLM_VEC3_ZERO_INIT;
        vec3 out_1 = GLM_VEC3_ZERO_INIT;
        vec3 out_2 = GLM_VEC3_ZERO_INIT;
        glm_project_zo(v0, mvp, camera->v_viewport, out_0);
        glm_project_zo(v1, mvp, camera->v_viewport, out_1);
        glm_project_zo(v2, mvp, camera->v_viewport, out_2);

        // Convert from 0.0 ~ 1.0 to 0 ~ UINT16_MAX
        uint32_t z0 = (uint32_t)(out_0[2]*(float)UINT16_MAX);
        uint32_t z1 = (uint32_t)(out_1[2]*(float)UINT16_MAX);
        uint32_t z2 = (uint32_t)(out_2[2]*(float)UINT16_MAX);

        // Check that the triangle vertices are in front of the camera (not behind)
        // Doing float compares for each pixel cuts FPS by half, this maybe could be
        // better: TODO
        if(((z0 > 0 && z0 < UINT16_MAX)) &&
           ((z1 > 0 && z1 < UINT16_MAX)) &&
           ((z2 > 0 && z2 < UINT16_MAX))){

            // Cast to int and see if any endpoints will be on screen
            int32_t x0 = (int32_t)out_0[0];
            int32_t y0 = (int32_t)out_0[1];

            int32_t x1 = (int32_t)out_1[0];
            int32_t y1 = (int32_t)out_1[1];

            int32_t x2 = (int32_t)out_2[0];
            int32_t y2 = (int32_t)out_2[1];

            uint16_t color = mesh_color->value;

            if(ivx/3 < triangle_colors->len){
                color = ((color_class_obj_t*)triangle_colors->items[ivx/3])->value;
            }

            engine_draw_filled_triangle_depth(color, x0, y0, z0, x1, y1, z1, x2, y2, z2, 1.0f, shader);

            // // Wireframe
            // bool endpoint_0_on_screen = engine_math_int32_between(x0, 0, SCREEN_WIDTH_MINUS_1) && engine_math_int32_between(y0, 0, SCREEN_HEIGHT_MINUS_1);
            // bool endpoint_1_on_screen = engine_math_int32_between(x1, 0, SCREEN_WIDTH_MINUS_1) && engine_math_int32_between(y1, 0, SCREEN_HEIGHT_MINUS_1);
            // bool endpoint_2_on_screen = engine_math_int32_between(x2, 0, SCREEN_WIDTH_MINUS_1) && engine_math_int32_between(y2, 0, SCREEN_HEIGHT_MINUS_1);

            // // If either endpoint is on screen, draw the full line
            // // This avoids drawing lines that are out of bounds on
            // // the camera's view plane and increases performance a
            // // ton
            // if(endpoint_0_on_screen || endpoint_1_on_screen){
            //     engine_draw_line(mesh_color->value, out_0[0], out_0[1], out_1[0], out_1[1], NULL, 1.0f, shader);
            // }

            // if(endpoint_1_on_screen || endpoint_2_on_screen){
            //     engine_draw_line(mesh_color->value, out_1[0], out_1[1], out_2[0], out_2[1], NULL, 1.0f, shader);
            // }

            // if(endpoint_2_on_screen || endpoint_0_on_screen){
            //     engine_draw_line(mesh_color->value, out_2[0], out_2[1], out_0[0], out_0[1], NULL, 1.0f, shader);
            // }
        }
    }
}


// Return `true` if handled loading the attr from internal structure, `false` otherwise
bool mesh_load_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_mesh_node_class_obj_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR_tick:
            destination[0] = self->tick_cb;
            destination[1] = self_node_base->attr_accessor;
            return true;
        break;
        case MP_QSTR_position:
            destination[0] = self->position;
            return true;
        break;
        case MP_QSTR_rotation:
            destination[0] = self->rotation;
            return true;
        break;
        case MP_QSTR_scale:
            destination[0] = self->scale;
            return true;
        break;
        case MP_QSTR_mesh:
            destination[0] = self->mesh;
            return true;
        break;
        case MP_QSTR_color:
            destination[0] = self->color;
            return true;
        break;
        case MP_QSTR_global_position:
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("ERROR: `global_position` is not supported on this node yet!"));
            return true;
        break;
        default:
            return false; // Fail
    }
}


// Return `true` if handled storing the attr from internal structure, `false` otherwise
bool mesh_store_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_mesh_node_class_obj_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR_tick:
            self->tick_cb = destination[1];
            return true;
        break;
        case MP_QSTR_position:
        {
            self->position = destination[1];
            mesh_node_set_translation(self);
            return true;
        }
        break;
        case MP_QSTR_rotation:
        {
            self->position = destination[1];
            mesh_node_set_rotation(self);
            return true;
        }
        break;
        case MP_QSTR_scale:
        {
            self->position = destination[1];
            mesh_node_set_scale(self);
            return true;
        }
        break;
        case MP_QSTR_mesh:
            self->mesh = destination[1];
            return true;
        break;
        case MP_QSTR_color:
            self->color = destination[1];
            return true;
        break;
        case MP_QSTR_global_position:
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("ERROR: `global_position` is not supported on this node yet!"));
            return true;
        break;
        default:
            return false; // Fail
    }
}


static mp_attr_fun_t mesh_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing MeshNode attr");
    node_base_attr_handler(self_in, attribute, destination,
                          (attr_handler_func[]){mesh_load_attr, node_base_load_attr},
                          (attr_handler_func[]){mesh_store_attr, node_base_store_attr}, 2);
    return mp_const_none;
}


/*  --- doc ---
    NAME: MeshNode
    ID: MeshNode
    DESC: Node that renders a list of vertices (without indices) (WIP). Note: 3D nodes do not currently support inheritance between each other, attributes like position, rotation, scale, and opacity will not work in parent/child inheritance.
    PARAM: [type={ref_link:Vector3}]             [name=position]                                    [value={ref_link:Vector3}]
    PARAM: [type=list]                           [name=vertices]                                    [value=list of {ref_link:Vector3}]
    PARAM:  [type=int]                           [name=layer]                                       [value=0 ~ 127]
    ATTR:  [type=function]                       [name={ref_link:add_child}]                        [value=function] 
    ATTR:  [type=function]                       [name={ref_link:get_child}]                        [value=function]
    ATTR:  [type=function]                       [name={ref_link:get_child_count}]                  [value=function]
    ATTR:  [type=function]                       [name={ref_link:node_base_mark_destroy}]           [value=function]
    ATTR:  [type=function]                       [name={ref_link:node_base_mark_destroy_all}]       [value=function]
    ATTR:  [type=function]                       [name={ref_link:node_base_mark_destroy_children}]  [value=function]
    ATTR:  [type=function]                       [name={ref_link:remove_child}]                     [value=function]
    ATTR:  [type=function]                       [name={ref_link:get_parent}]                       [value=function]
    ATTR:  [type=function]                       [name={ref_link:tick}]                             [value=function]
    ATTR:  [type={ref_link:Vector3}]             [name=position]                                    [value={ref_link:Vector3}]
    ATTR:  [type={ref_link:Vector3}]             [name=rotation]                                    [value={ref_link:Vector3}]
    ATTR:  [type={ref_link:Vector3}]             [name=scale]                                       [value={ref_link:Vector3}]
    ATTR:  [type=list]                           [name=vertices]                                    [value=list of {ref_link:Vector3}]
    OVRR:  [type=function]                       [name={ref_link:tick}]                             [value=function]
*/
mp_obj_t mesh_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New MeshNode");
    
    // This node uses a depth buffer to be drawn correctly
    engine_display_check_depth_buffer_created();

    mp_arg_t allowed_args[] = {
        { MP_QSTR_child_class,  MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_position,     MP_ARG_OBJ, {.u_obj = vector3_class_new(&vector3_class_type, 3, 0, (mp_obj_t[]){mp_obj_new_float(0.0f), mp_obj_new_float(0.0f), mp_obj_new_float(0.0f)})} },
        { MP_QSTR_rotation,     MP_ARG_OBJ, {.u_obj = vector3_class_new(&vector3_class_type, 3, 0, (mp_obj_t[]){mp_obj_new_float(0.0f), mp_obj_new_float(0.0f), mp_obj_new_float(0.0f)})} },
        { MP_QSTR_scale,        MP_ARG_OBJ, {.u_obj = vector3_class_new(&vector3_class_type, 3, 0, (mp_obj_t[]){mp_obj_new_float(1.0f), mp_obj_new_float(1.0f), mp_obj_new_float(1.0f)})} },
        { MP_QSTR_mesh,         MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_color,        MP_ARG_OBJ, {.u_obj = MP_OBJ_NEW_SMALL_INT(0xffff)} },
    };
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    enum arg_ids {child_class, position, rotation, scale, mesh, color};
    bool inherited = false;

    // If there is one positional argument and it isn't the first 
    // expected argument (as is expected when using positional
    // arguments) then define which way to parse the arguments
    if(n_args >= 1 && mp_obj_get_type(args[0]) != &vector3_class_type){
        // Using positional arguments but the type of the first one isn't
        // as expected. Must be the child class
        mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args), allowed_args, parsed_args);
        inherited = true;
    }else{
        // Whether we're using positional arguments or not, prase them this
        // way. It's a requirement that the child class be passed using position.
        // Adjust what and where the arguments are parsed, since not inherited based
        // on the first argument
        mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args)-1, allowed_args+1, parsed_args+1);
        inherited = false;
    }

    // All nodes are a engine_node_base_t node. Specific node data is stored in engine_node_base_t->node
    engine_node_base_t *node_base = mp_obj_malloc_with_finaliser(engine_node_base_t, &engine_mesh_node_class_type);
    node_base_init(node_base, &engine_mesh_node_class_type, NODE_TYPE_MESH_3D, 0);

    engine_mesh_node_class_obj_t *mesh_node = m_malloc(sizeof(engine_mesh_node_class_obj_t));
    node_base->node = mesh_node;
    node_base->attr_accessor = node_base;

    mesh_node->tick_cb = mp_const_none;

    mesh_node->position = parsed_args[position].u_obj;
    mesh_node->rotation = parsed_args[rotation].u_obj;
    mesh_node->scale    = parsed_args[scale].u_obj;
    mesh_node->mesh     = parsed_args[mesh].u_obj;
    mesh_node->color    = engine_color_wrap(parsed_args[color].u_obj);

    vector3_class_obj_t *p = (vector3_class_obj_t*)mesh_node->position;
    vector3_class_obj_t *r = (vector3_class_obj_t*)mesh_node->rotation;
    vector3_class_obj_t *s = (vector3_class_obj_t*)mesh_node->scale;

    glm_translate_make(mesh_node->m_translation, (vec3){p->x.value, p->y.value, p->z.value});
    glm_euler((vec3){r->x.value, r->y.value, r->z.value}, mesh_node->m_rotation);
    glm_scale_make(mesh_node->m_scale, (vec3){s->x.value, s->y.value, s->z.value});

    p->on_changed = &mesh_node_set_translation;
    p->on_change_user_ptr = mesh_node;

    r->on_changed = &mesh_node_set_rotation;
    r->on_change_user_ptr = mesh_node;

    s->on_changed = &mesh_node_set_scale;
    s->on_change_user_ptr = mesh_node;

    if(inherited == true){
        // Get the Python class instance
        mp_obj_t node_instance = parsed_args[child_class].u_obj;

        // Because the instance doesn't have a `node_base` yet, restore the
        // instance type original attr function for now (otherwise get core abort)
        node_base_set_attr_handler_default(node_instance);

        // Look for function overrides otherwise use the defaults
        mp_obj_t dest[2];
        mp_load_method_maybe(node_instance, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            mesh_node->tick_cb = mp_const_none;
        }else{                                                  // Likely found method (could be attribute)
            mesh_node->tick_cb = dest[0];
        }

        // Store one pointer on the instance. Need to be able to get the
        // node base that contains a pointer to the engine specific data we
        // care about
        // mp_store_attr(node_instance, MP_QSTR_node_base, node_base);
        mp_store_attr(node_instance, MP_QSTR_node_base, node_base);

        // Store default Python class instance attr function
        // and override with custom intercept attr function
        // so that certain callbacks/code can run (see py/objtype.c:mp_obj_instance_attr(...))
        node_base_set_attr_handler(node_instance, mesh_node_class_attr);

        // Need a way to access the object node instance instead of the native type for callbacks (tick, draw, collision)
        node_base->attr_accessor = node_instance;
    }

    return MP_OBJ_FROM_PTR(node_base);
}


// Class attributes
static const mp_rom_map_elem_t mesh_node_class_locals_dict_table[] = {

};
static MP_DEFINE_CONST_DICT(mesh_node_class_locals_dict, mesh_node_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    engine_mesh_node_class_type,
    MP_QSTR_MeshNode,
    MP_TYPE_FLAG_NONE,

    make_new, mesh_node_class_new,
    attr, mesh_node_class_attr,
    locals_dict, &mesh_node_class_locals_dict
);