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
#include <gol/world_json.hpp>
#include <gol/world_scene_node.hpp>

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

    linear_scene scene;
    scene.push<skybox_scene_node>("/home/missed/code/gol/assets/skybox/skybox.model");
    scene.push<world_scene_node>();
    // scene.push<model_scene_node>("/home/missed/code/gol/assets/sasuke/sasuke.model");

    f32 delta_time{ 0 };
    f32 last_ticks{ 0 };

    for(;;)
    {
        SDL_Event ev;
        while (SDL_PollEvent(&ev))
        {
            im::handle_event(ev);
            switch (ev.type)
            {
                case SDL_EVENT_QUIT: return EXIT_SUCCESS;
                case SDL_EVENT_WINDOW_RESIZED:
                    glViewport(0, 0, ev.window.data1, ev.window.data2);
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
