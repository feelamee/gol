#include <engine/scene.hpp>
#include <engine/log.hpp>

namespace gol
{

constexpr static auto model_vertex_shader_src = R"(
#version 320 es

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;

layout (location = 3) uniform mat4 model;
layout (location = 4) uniform mat4 view;
layout (location = 5) uniform mat4 projection;

layout (location = 10) out vec2 out_uv;

void main()
{
    vec4 pos = vec4(position, 1.0f);
    gl_Position = projection * view * model * pos;
    out_uv = vec2(uv.x, 1.0f - uv.y);
}
)";

constexpr static auto model_fragment_shader_src = R"(
#version 320 es
precision mediump float;

layout (location = 10) in vec2 uv;

layout (location = 6) uniform sampler2D tex;

out vec4 out_color;

void main()
{
    out_color = texture(tex, uv);
}
)";

constexpr static auto skybox_vertex_shader_src = R"(
#version 320 es
precision mediump float;

layout (location = 0) in vec3 pos;

layout (location = 1) uniform mat4 view;
layout (location = 2) uniform mat4 projection;

layout (location = 10) out vec3 out_uv;

void main()
{
    out_uv = pos;
    gl_Position = (projection * mat4(mat3(view)) * vec4(pos, 1.0)).xyww;
}
)";

constexpr static auto skybox_fragment_shader_src = R"(
#version 320 es
precision mediump float;

layout (location = 10) in vec3 uv;

layout (location = 3) uniform samplerCube cubemap;

out vec4 out_color;

void main()
{
    out_color = texture(cubemap, uv);
}
)";


void scene_node::handle_event(SDL_Event)
{
}

void scene_node::simulate(float delta)
{
}

void scene_node::draw(draw_info const & di) const
{
}

mat4 scene_node::model_mat() const
{
    if (dirty_transform)
    {
        transform = scale(
            rotate(
                translate(mat4{ 1.0f }, pos),
                rotation,
                vec3{ 0.0f, 0.0f, -1.0f }
            ),
            vec3{ scaling }
        );
        dirty_transform = false;
    }

    return transform;
}

void scene_node::set_pos(vec3 p)
{
    dirty_transform = true;
    pos = p;
}

void scene_node::set_scaling(f32 s)
{
    dirty_transform = true;
    scaling = s;
}

model_scene_node::model_scene_node()
{
    shader.id = glCreateProgram();
    gl::attach_source(shader, gl::stage::fragment, model_fragment_shader_src);
    gl::attach_source(shader, gl::stage::vertex, model_vertex_shader_src);
    gl::link(shader);
}

model_scene_node::model_scene_node(std::filesystem::path const& model_path)
    : model_scene_node()
{
    set_model_path(model_path);
}

void model_scene_node::draw(draw_info const & di) const
{
    if (!model.has_value())
        return;

    using gl::bind, gl::bind_guard;

    bind_guard g1(shader);

    bind(3, model_mat());
    bind(4, di.view);
    bind(5, di.projection);
    bind(6, model);

    glDrawElements(GL_TRIANGLES, GLsizei(model.indices_count), GL_UNSIGNED_INT, nullptr);
}

bool model_scene_node::set_model_path(std::filesystem::path const& model_path)
{
    if (!from_file(model, 0, model_path))
    {
        log::err("can't load model: {}", model_path.string());
        return false;
    }

    return true;
}


skybox_scene_node::skybox_scene_node()
{
    shader.id = glCreateProgram();
    gl::attach_source(shader, gl::stage::fragment, skybox_fragment_shader_src);
    gl::attach_source(shader, gl::stage::vertex, skybox_vertex_shader_src);
    gl::link(shader);

}

skybox_scene_node::skybox_scene_node(std::filesystem::path const& model_path)
    : skybox_scene_node()
{
    set_model_path(model_path);
}


skybox_scene_node::~skybox_scene_node()
{
    destroy(shader);
    destroy(model);
}

void skybox_scene_node::draw(draw_info const& di) const
{
    if (!model.has_value())
        return; // TODO! maybe add fallback model to draw it whenever actual model not available?

    using gl::bind, gl::bind_guard;

    GLint depth_func;
    glGetIntegerv(GL_DEPTH_FUNC, &depth_func);

    glDepthFunc(GL_LEQUAL);
    bind_guard g1(shader);
    bind_guard g2(3, model);

    bind(1, di.view);
    bind(2, di.projection);

    glDrawElements(GL_TRIANGLES, GLsizei(model.indices_count), GL_UNSIGNED_INT, nullptr);
    glDepthFunc(GLenum(depth_func));
}

bool skybox_scene_node::set_model_path(std::filesystem::path const& model_path)
{
    if (!from_file(model, 0, model_path))
    {
        log::err("can't load model: {}", model_path.string());
        return false;
    }

    return true;
}

void linear_scene::handle_event(SDL_Event const & ev)
{
    cam.handle_event(ev);
    for (auto & o : objects)
        o->handle_event(ev);
}

void linear_scene::simulate(float delta)
{
    cam.simulate(delta);
    for (auto & o : objects)
        o->simulate(delta);
}

void linear_scene::draw(draw_info const & di) const
{
    scene_node::draw_info obj_di{
        .view = cam.view(),
        .projection = di.projection,
    };

    // cam.draw(obj_di);
    for (auto const & o : objects)
        o->draw(obj_di);
}

}
