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

#define CGLM_CLIPSPACE_INCLUDE_ALL 1
#include "../lib/cglm/include/cglm/cglm.h"
#include "../lib/cglm/include/cglm/vec3.h"
#include "../lib/cglm/include/cglm/mat4.h"
#include "../lib/cglm/include/cglm/cam.h"


// Defined in camera_node.h
extern const vector3_class_obj_t world_north;


void mesh_node_class_draw(mp_obj_t mesh_node_base_obj, mp_obj_t camera_node){
    engine_node_base_t *mesh_node_base = mesh_node_base_obj;
    engine_mesh_node_class_obj_t *mesh_node = mesh_node_base->node;

    engine_node_base_t *camera_node_base = camera_node;
    engine_camera_node_class_obj_t *camera = camera_node_base->node;

    engine_shader_t *shader = engine_get_builtin_shader(EMPTY_SHADER);

    vector3_class_obj_t *camera_position = camera->position;
    float camera_view_distance = mp_obj_get_float(camera->view_distance);

    vec3 cam_position = {camera_position->x.value, camera_position->y.value, camera_position->z.value};
    vec3 cam_target = GLM_VEC3_ZERO_INIT;
    vec3 cam_up = {world_north.x.value, world_north.y.value, world_north.z.value};

    if(mesh_node->vertices->len < 3){
        return;
    }

    mat4 m_view = GLM_MAT4_ZERO_INIT;
    glm_lookat(cam_position, cam_target, cam_up, m_view);

    mat4 m_projection = GLM_MAT4_ZERO_INIT;
    glm_perspective(1.571f, SCREEN_WIDTH/SCREEN_HEIGHT, 0.5f, camera_view_distance, m_projection);

    // mat4 m_model = GLM_MAT4_IDENTITY_INIT;

    mat4 mvp = GLM_MAT4_ZERO_INIT;
    glm_mat4_mul(m_projection, m_view, mvp);
    // glm_mat4_mul(mvp, m_model, mvp);

    vec4 v_viewport = {0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT};


    for(uint16_t ivx=0; ivx<mesh_node->vertices->len; ivx+=3){
        vector3_class_obj_t *vertex_0 = mesh_node->vertices->items[ivx];
        vector3_class_obj_t *vertex_1 = mesh_node->vertices->items[ivx+1];
        vector3_class_obj_t *vertex_2 = mesh_node->vertices->items[ivx+2];

        vec3 v0 = {vertex_0->x.value, vertex_0->y.value, vertex_0->z.value};
        vec3 v1 = {vertex_1->x.value, vertex_1->y.value, vertex_1->z.value};
        vec3 v2 = {vertex_2->x.value, vertex_2->y.value, vertex_2->z.value};

        vec3 out_0 = GLM_VEC3_ZERO_INIT;
        vec3 out_1 = GLM_VEC3_ZERO_INIT;
        vec3 out_2 = GLM_VEC3_ZERO_INIT;
        glm_project_zo(v0, mvp, v_viewport, out_0);
        glm_project_zo(v1, mvp, v_viewport, out_1);
        glm_project_zo(v2, mvp, v_viewport, out_2);

        float z0 = out_0[2];
        float z1 = out_1[2];
        float z2 = out_2[2];

        // Check that the triangle vertices are in front of the camera (not behind)
        // Doing float compares for each pixel cuts FPS by half, this maybe could be
        // better: TODO
        if(((z0 > 0.0f && z0 < 1.0f)) &&
           ((z1 > 0.0f && z1 < 1.0f)) &&
           ((z2 > 0.0f && z2 < 1.0f))){

            // Cast to int and see if any endpoints will be on screen
            int32_t x0 = (int32_t)out_0[0];
            int32_t y0 = (int32_t)out_0[1];

            int32_t x1 = (int32_t)out_1[0];
            int32_t y1 = (int32_t)out_1[1];

            int32_t x2 = (int32_t)out_2[0];
            int32_t y2 = (int32_t)out_2[1];

            engine_draw_filled_triangle(0b1111100000000000, x0, y0, x1, y1, x2, y2, 1.0f, shader);

            // // Wireframe
            // bool endpoint_0_on_screen = engine_math_int32_between(x0, 0, SCREEN_WIDTH_MINUS_1) && engine_math_int32_between(y0, 0, SCREEN_HEIGHT_MINUS_1);
            // bool endpoint_1_on_screen = engine_math_int32_between(x1, 0, SCREEN_WIDTH_MINUS_1) && engine_math_int32_between(y1, 0, SCREEN_HEIGHT_MINUS_1);
            // bool endpoint_2_on_screen = engine_math_int32_between(x2, 0, SCREEN_WIDTH_MINUS_1) && engine_math_int32_between(y2, 0, SCREEN_HEIGHT_MINUS_1);

            // // If either endpoint is on screen, draw the full line
            // // This avoids drawing lines that are out of bounds on
            // // the camera's view plane and increases performance a
            // // ton
            // if(endpoint_0_on_screen || endpoint_1_on_screen){
            //     engine_draw_line(0xffff, out_0[0], out_0[1], out_1[0], out_1[1], NULL, 1.0f, &empty_shader);
            // }

            // if(endpoint_1_on_screen || endpoint_2_on_screen){
            //     engine_draw_line(0xffff, out_1[0], out_1[1], out_2[0], out_2[1], NULL, 1.0f, &empty_shader);
            // }

            // if(endpoint_2_on_screen || endpoint_0_on_screen){
            //     engine_draw_line(0xffff, out_2[0], out_2[1], out_0[0], out_0[1], NULL, 1.0f, &empty_shader);
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
        case MP_QSTR_vertices:
            destination[0] = self->vertices;
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
            self->position = destination[1];
            return true;
        break;
        case MP_QSTR_vertices:
            self->vertices = destination[1];
            return true;
        break;
        default:
            return false; // Fail
    }
}


static mp_attr_fun_t mesh_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing MeshNode attr");
    node_base_attr_handler(self_in, attribute, destination,
                          (attr_handler_func[]){node_base_load_attr, mesh_load_attr},
                          (attr_handler_func[]){node_base_store_attr, mesh_store_attr}, 2);
    return mp_const_none;
}


/*  --- doc ---
    NAME: MeshNode
    ID: MeshNode
    DESC: Node that renders a list of vertices (without indices)
    PARAM: [type={ref_link:Vector3}]             [name=position]                                    [value={ref_link:Vector3}]
    PARAM: [type=list]                           [name=vertices]                                    [value=list of {ref_link:Vector3}]
    ATTR:  [type=function]                       [name={ref_link:add_child}]                        [value=function] 
    ATTR:  [type=function]                       [name={ref_link:get_child}]                        [value=function]
    ATTR:  [type=function]                       [name={ref_link:get_child_count}]                  [value=function]
    ATTR:  [type=function]                       [name={ref_link:node_base_mark_destroy}]           [value=function]
    ATTR:  [type=function]                       [name={ref_link:node_base_mark_destroy_all}]       [value=function]
    ATTR:  [type=function]                       [name={ref_link:node_base_mark_destroy_children}]  [value=function]
    ATTR:  [type=function]                       [name={ref_link:remove_child}]                     [value=function]
    ATTR:  [type=function]                       [name={ref_link:tick}]                             [value=function]
    ATTR:  [type={ref_link:Vector3}]             [name=position]                                    [value={ref_link:Vector3}]
    ATTR:  [type=list]                           [name=vertices]                                    [value=list of {ref_link:Vector3}]
    ATTR:  [type=int]                            [name=layer]                                       [value=0 ~ 127]
    OVRR:  [type=function]                       [name={ref_link:tick}]                             [value=function]
*/
mp_obj_t mesh_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){

    // This node uses a depth buffer to be drawn correctly
    engine_display_check_depth_buffer_created();

    ENGINE_INFO_PRINTF("New MeshNode");

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_child_class,  MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_position,     MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_vertices,     MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    enum arg_ids {child_class, position, vertices};
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

    if(parsed_args[position].u_obj == MP_OBJ_NULL) parsed_args[position].u_obj = vector3_class_new(&vector3_class_type, 3, 0, (mp_obj_t[]){mp_obj_new_float(0.0f), mp_obj_new_float(0.0f), mp_obj_new_float(0.0f)});
    if(parsed_args[vertices].u_obj == MP_OBJ_NULL) parsed_args[vertices].u_obj = mp_obj_new_list(0, NULL);

    // All nodes are a engine_node_base_t node. Specific node data is stored in engine_node_base_t->node
    engine_node_base_t *node_base = mp_obj_malloc_with_finaliser(engine_node_base_t, &engine_mesh_node_class_type);
    node_base_init(node_base, &engine_mesh_node_class_type, NODE_TYPE_MESH_3D);

    engine_mesh_node_class_obj_t *mesh_node = m_malloc(sizeof(engine_mesh_node_class_obj_t));
    node_base->node = mesh_node;
    node_base->attr_accessor = node_base;

    mesh_node->tick_cb = mp_const_none;

    mesh_node->position = parsed_args[position].u_obj;
    mesh_node->vertices = parsed_args[vertices].u_obj;

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