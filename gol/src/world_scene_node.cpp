#include <gol/world_scene_node.hpp>

constexpr static auto world_vertex_shader_src = R"(
#version 320 es

layout(std430, binding = 0) readonly buffer world_data_buffer
{
    uint world_data[];  // true - alive, false - dead
};

layout (location = 0) uniform vec4 alive_color;
layout (location = 1) uniform vec4 dead_color;
layout (location = 2) uniform uvec2 size; // row_count, column_count

layout (location = 3) uniform mat4 model;
layout (location = 4) uniform mat4 view;
layout (location = 5) uniform mat4 projection;

layout (location = 10) out vec4 out_color;

const vec3 cube_positions[8] = vec3[8](
    vec3(-0.5,-0.5, 0.5),
    vec3( 0.5,-0.5, 0.5),
    vec3( 0.5, 0.5, 0.5),
    vec3(-0.5, 0.5, 0.5),
    vec3(-0.5,-0.5,-0.5),
    vec3( 0.5,-0.5,-0.5),
    vec3( 0.5, 0.5,-0.5),
    vec3(-0.5, 0.5,-0.5)
);

const uint mask[4] = uint[4](
    uint(0x000000FF),
    uint(0x0000FF00),
    uint(0x00FF0000),
    uint(0xFF000000)
);

const float cube_size = 1.0f;
const float cubes_margin = 0.5f;

void main()
{
    uint row_count = size.x;
    uint column_count = size.y;

    float row = float(uint(gl_InstanceID) / column_count);
    float col = float(uint(gl_InstanceID) % column_count);

    vec4 pos = vec4(cube_positions[gl_VertexID], 1.0f);
    pos.x += (col - (float(column_count) - 1.0f) / 2.0f) * (cube_size + cubes_margin);
    pos.z += (row - (float(row_count)    - 1.0f) / 2.0f) * (cube_size + cubes_margin);
    gl_Position = projection * view * model * pos;

    uint is_alive = world_data[gl_InstanceID / 4] & mask[gl_InstanceID % 4];
    out_color = is_alive != 0u ? alive_color : dead_color;
}
)";

constexpr static auto world_fragment_shader_src = R"(
#version 320 es
precision mediump float;

layout (location = 10) in vec4 color;

out vec4 out_color;

void main()
{
    out_color = color;
}
)";

namespace gol
{

world_scene_node::world_scene_node()
    : world_scene_node(world{})
{
}

world_scene_node::world_scene_node(world w)
    : gol_world(std::move(w))
{
    gl::from_mem(GL_SHADER_STORAGE_BUFFER, gpu_world, gol_world.data);

    constexpr std::array<GLuint, 36> cube_indices = {
        0, 1, 2,  2, 3, 0, // front
        4, 5, 6,  6, 7, 4, // back
        4, 0, 3,  3, 7, 4, // left
        1, 5, 6,  6, 2, 1, // right
        3, 2, 6,  6, 7, 3, // up
        0, 1, 5,  5, 4, 0  // bottom
    };
    gl::from_mem(GL_ELEMENT_ARRAY_BUFFER, ebo, cube_indices);

    glGenVertexArrays(1, &vao.id);
    bind(vao, ebo);

    shader.id = glCreateProgram();
    gl::attach_source(shader, gl::stage::fragment, world_fragment_shader_src);
    gl::attach_source(shader, gl::stage::vertex, world_vertex_shader_src);
    gl::link(shader);
}

world_scene_node::~world_scene_node()
{
    destroy(gpu_world);
    destroy(shader);
}

void world_scene_node::handle_event(SDL_Event)
{
}

void world_scene_node::simulate(delta_type)
{
}

void world_scene_node::draw(draw_info const & di) const
{
    if (gol_world.empty())
        return;

    using gl::bind, gl::bind_guard;

    bind_guard g1(shader);
    bind_guard g2(vao);
    bind_guard g3(0, gpu_world);

    bind(0, vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
    bind(1, vec4{ 0.0f, 0.0f, 0.0f, 1.0f });
    bind(2, uvec2{ gol_world.row_count, gol_world.column_count });
    bind(3, model_mat());
    bind(4, di.view);
    bind(5, di.projection);

    glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr, GLsizei(gol_world.data.size()));
}

}

