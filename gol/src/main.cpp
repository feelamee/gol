#include <engine/fundamental.hpp>
#include <engine/context.hpp>
#include <engine/error.hpp>
#include <engine/sdl.hpp>
#include <engine/sdl_dialog.hpp>
#include <engine/ranges.hpp>
#include <engine/mesh.hpp>
#include <engine/image.hpp>
#include <engine/camera.hpp>
#include <engine/gl.hpp>
#include <engine/algorithm.hpp>
#include <engine/imgui.hpp>
#include <engine/log.hpp>
#include <engine/log_macro.hpp>
#include <engine/scene.hpp>
using namespace gol;

#include <gol/world.hpp>

#include <SDL3/SDL.h>

#include <glad/glad.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>
#pragma GCC diagnostic pop

#include <imgui/imgui.h>

#include <cstdlib>
#include <csignal>

constexpr auto vertex_shader_src = R"(
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

constexpr auto fragment_shader_src = R"(
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

constexpr auto skybox_vertex_shader_src = R"(
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

constexpr auto skybox_fragment_shader_src = R"(
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

int main()
{
    // register custom SIGINT to allow Ctrl+C immediately close program even if assets loading...
    std::signal(
        SIGINT,
        [](int)
        {
            log::err("SIGINT is called\n");
            std::quick_exit(EXIT_FAILURE);
        }
    );

    context ctx;
    glEnable(GL_DEPTH_TEST);

    gl::shader model_shader_program{ glCreateProgram() };
    gl::attach_source(model_shader_program, gl::stage::fragment, fragment_shader_src);
    gl::attach_source(model_shader_program, gl::stage::vertex, vertex_shader_src);
    gl::link(model_shader_program);
    GOL_SCOPE_EXIT { destroy(model_shader_program); };

    constexpr auto model_filepath{ "/home/missed/code/gol/assets/sasuke/sasuke.model" };
    gl::model sasuke_model;
    if (!from_file(sasuke_model, 0, model_filepath))
        throw error{ "[ERROR][ENGINE] can't load model: {}", model_filepath };

    GOL_SCOPE_EXIT { destroy(sasuke_model); };

    gl::shader skybox_shader_program{ glCreateProgram() };
    gl::attach_source(skybox_shader_program, gl::stage::fragment, skybox_fragment_shader_src);
    gl::attach_source(skybox_shader_program, gl::stage::vertex, skybox_vertex_shader_src);
    gl::link(skybox_shader_program);
    GOL_SCOPE_EXIT { destroy(skybox_shader_program); };

    constexpr auto skybox_filepath{ "/home/missed/code/gol/assets/skybox/skybox.model" };
    gl::model skybox_model;
    if (!from_file(skybox_model, 0, skybox_filepath))
        throw error{ "[ERROR][ENGINE] can't load model: {}", skybox_filepath };

    linear_scene scene;
    // scene.push<model_scene_node>(sasuke_model, model_shader_program);
    scene.push<skybox_scene_node>(skybox_model, skybox_shader_program);

    f32 delta_time{ 0 };
    f32 last_ticks{ 0 };

    struct
    {
        bool show_demo_window{ false };
    } imgui_state;

    for(;;)
    {
        SDL_Event ev;
        while (SDL_PollEvent(&ev))
        {
            im::handle_event(ev);

            if (rng::contains({
                SDL_EVENT_KEY_DOWN,
                SDL_EVENT_KEY_UP,
                SDL_EVENT_TEXT_EDITING,
                SDL_EVENT_TEXT_INPUT,
            }, ev.key.type))
            {
                if (ImGui::GetIO().WantCaptureKeyboard)
                    continue;
            }

            if (rng::contains({
                SDL_EVENT_MOUSE_MOTION,
                SDL_EVENT_MOUSE_BUTTON_DOWN,
                SDL_EVENT_MOUSE_BUTTON_UP,
                SDL_EVENT_MOUSE_WHEEL,
            }, ev.key.type))
            {
                if (ImGui::GetIO().WantCaptureMouse)
                    continue;
            }

            switch (ev.type)
            {
                case SDL_EVENT_QUIT: return EXIT_SUCCESS;
                case SDL_EVENT_WINDOW_RESIZED:
                    glViewport(0, 0, ev.window.data1, ev.window.data2);
                    break;

                case SDL_EVENT_KEY_UP:
                    switch (ev.key.key)
                    {
                        case SDLK_GRAVE:
                            imgui_state.show_demo_window = !imgui_state.show_demo_window;
                            break;

                        case SDLK_O:
                            if (ev.key.mod & SDL_KMOD_CTRL)
                            {
                                select_file_dialog(
                                    [&](auto const& files)
                                    {
                                        for (auto const& f : files)
                                        {
                                            gl::model model;
                                            if (!from_file(model, 0, f))
                                            {
                                                log::err("[ERROR][ENGINE] can't load model: {}", f);
                                                continue;
                                            }

                                            scene.push<model_scene_node>(model, model_shader_program);
                                        }
                                    },
                                    { .many = true }
                                );
                            }
                    }
                    break;
            }

            scene.handle_event(ev);
        }

        scene.simulate(delta_time);

        int w = 960, h = 540;
        if (!SDL_GetWindowSize(ctx.window, &w, &h))
            sdl::log_error();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        scene.draw({
            .projection = perspective(radians(45.0f), f32(w) / f32(h), 0.01f, 100.0f)
        });

        {
            im::frame();

            if (imgui_state.show_demo_window)
                ImGui::ShowDemoWindow(&imgui_state.show_demo_window);

            im::render();
        }

        if (!SDL_GL_SwapWindow(ctx.window))
            sdl::log_error();

        {
            SDL_Time ticks{};
            if (!SDL_GetCurrentTime(&ticks))
                sdl::log_error();
            f32 fticks = f32(ticks % 360'000'000'000) / 1E8f;

            delta_time = fticks - last_ticks;
            last_ticks = fticks;
        }
    }

    return EXIT_SUCCESS;
}
