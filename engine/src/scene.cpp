#include <engine/scene.hpp>
#include <engine/log.hpp>
#include <engine/sdl.hpp>
#include <engine/context.hpp>

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


void scene_node::handle_event(SDL_Event ev)
{
    if (enabled())
        do_handle_event(ev);
}

void scene_node::simulate(delta_type delta)
{
    if (enabled())
        do_simulate(delta);
}

void scene_node::draw(draw_info const & di) const
{
    if (visible())
        return do_draw(di);
}

void scene_node::do_handle_event(SDL_Event)
{
}

void scene_node::do_simulate(delta_type)
{
}

void scene_node::do_draw(draw_info const &) const
{
}

mat4 scene_node::model_mat() const
{
    if (dirty_transform)
    {
        transform = scale(
            rotate(
                translate(mat4{ 1.0f }, position),
                rotation,
                vec3{ 0.0f, 0.0f, -1.0f }
            ),
            vec3{ scaling }
        );
        dirty_transform = false;
    }

    return transform;
}

vec3 scene_node::get_position() const
{
    return position;
}

void scene_node::set_position(vec3 p)
{
    dirty_transform = true;
    position = p;
}

void scene_node::enable(bool e)
{
    is_enabled = e;
}

bool scene_node::enabled() const
{
    return is_enabled;
}

void scene_node::visible(bool v)
{
    is_visible = v;
}

bool scene_node::visible() const
{
    return is_visible;
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

void model_scene_node::do_draw(draw_info const & di) const
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

void skybox_scene_node::do_draw(draw_info const& di) const
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


camera_scene_node::camera_scene_node()
    : camera_scene_node({ 0.0f, 0, 0 }, { 0.0f, 0, -1 }, { 0.0f, 1, 0 })
{
}

camera_scene_node::camera_scene_node(vec3 const& pos, vec3 const& dir, vec3 const& up)
    : init_position{ pos }
    , init_direction{ dir }
    , world_up{ up }
    , direction{ dir }
{
    set_position(pos);
    update_vectors();
}

void camera_scene_node::do_handle_event(SDL_Event ev)
{
    SDL_Window * window = gol::ctx().window;

    switch (ev.type)
    {
        case SDL_EVENT_KEY_DOWN:
        {
            switch (ev.key.key)
            {
            case SDLK_W: is_w = is_mouse_middle_button; break;
            case SDLK_S: is_s = is_mouse_middle_button; break;
            case SDLK_A: is_a = is_mouse_middle_button; break;
            case SDLK_D: is_d = is_mouse_middle_button; break;
            }

            is_lshift = ev.key.mod & SDL_KMOD_LSHIFT;
        }
        break;

        case SDL_EVENT_KEY_UP:
        {
            switch (ev.key.key)
            {
            case SDLK_W: is_w = false; break;
            case SDLK_S: is_s = false; break;
            case SDLK_A: is_a = false; break;
            case SDLK_D: is_d = false; break;
            case SDLK_EQUALS:
                set_position(init_position);
                yaw = init_yaw;
                pitch = init_pitch;
                update_vectors();
                break;
            }

            is_lshift = ev.key.mod & SDL_KMOD_LSHIFT;
        }
        break;

        case SDL_EVENT_MOUSE_MOTION:
        if (is_mouse_middle_button)
        {
            pitch -= ev.motion.yrel * 0.05f;
            yaw += ev.motion.xrel * 0.05f;

            pitch = std::clamp(pitch, -89.0f, 89.0f);

            update_vectors();
        }
        break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (ev.button.button == 2)
        {
            if (!SDL_SetWindowRelativeMouseMode(window, true))
                sdl::log_error();

            SDL_HideCursor();

            int w, h;
            SDL_GetWindowSize(window, &w, &h);
            SDL_WarpMouseInWindow(window, f32(w) / 2, f32(h) / 2);

            is_mouse_middle_button = true;
        }
        break;

        case SDL_EVENT_MOUSE_BUTTON_UP:
        if (ev.button.button == 2)
        {
            int w, h;
            SDL_GetWindowSize(window, &w, &h);
            SDL_WarpMouseInWindow(window, f32(w) / 2, f32(h) / 2);

            if (!SDL_SetWindowRelativeMouseMode(window, false))
                sdl::log_error();

            SDL_ShowCursor();

            is_mouse_middle_button = false;
        }
        break;
    }
}

void camera_scene_node::do_simulate(delta_type delta)
{
    f32 const real_speed = speed * (is_lshift ? 5.0f : 1.0f);
    f32 const distance = real_speed * std::chrono::duration_cast<std::chrono::duration<float>>(delta).count();
    if (is_w)
        set_position(get_position() + direction * distance);
    if (is_s)
        set_position(get_position() - direction * distance);
    if (is_a)
        set_position(get_position() - right * distance);
    if (is_d)
        set_position(get_position() + right * distance);
}

mat4 camera_scene_node::view() const
{
    return lookAt(get_position(), get_position() + direction, up);
}

void camera_scene_node::update_vectors()
{
    direction[0] = std::cos(radians(pitch)) * std::cos(radians(yaw));
    direction[1] = std::sin(radians(pitch));
    direction[2] = std::cos(radians(pitch)) * std::sin(radians(yaw));

    direction = normalize(direction);
    right = normalize(cross(direction, world_up));
    up = normalize(cross(right, direction));
}


void linear_scene::do_handle_event(SDL_Event ev)
{
    for (auto & o : objects)
        o->handle_event(ev);
}

void linear_scene::do_simulate(delta_type delta)
{
    for (auto & o : objects)
        o->simulate(delta);
}

void linear_scene::do_draw(draw_info const & di) const
{
    // TODO! maybe better pass by mutable ref? or by value
    auto di2 = di;
    if (camera)
        di2.view = camera->view();

    for (auto const & o : objects)
        o->draw(di2);
}

}
