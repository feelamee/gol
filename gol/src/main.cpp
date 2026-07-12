#include <engine/fundamental.hpp>
#include <engine/context.hpp>
#include <engine/error.hpp>
#include <engine/sdl.hpp>
#include <engine/sdl_dialog.hpp>
#include <engine/ranges.hpp>
#include <engine/mesh.hpp>
#include <engine/image.hpp>
#include <engine/gl.hpp>
#include <engine/algorithm.hpp>
#include <engine/imgui.hpp>
#include <engine/log.hpp>
#include <engine/log_macro.hpp>
#include <engine/scene.hpp>
#include <engine/ticker.hpp>
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
#include <fstream>

struct input_data
{
    world input_world = {
        5, 5,
        {
            cell::dead, cell::dead, cell::dead,  cell::dead, cell::dead,
            cell::dead, cell::dead, cell::alive, cell::dead, cell::dead,
            cell::dead, cell::dead, cell::alive, cell::dead, cell::dead,
            cell::dead, cell::dead, cell::alive, cell::dead, cell::dead,
            cell::dead, cell::dead, cell::dead,  cell::dead, cell::dead,
        }
    };
    struct simulation
    {
        u32 fps = 60;

        constexpr inline static u32 infinite_cycles = u32(-1);
        u32 cycles = infinite_cycles;
    } input_simulation;
};

void from_json(nlohmann::json const& j, input_data::simulation & r)
{
    if (!j.at("fps").is_number_unsigned())
        throw error{ "key 'fps' must have unsigned type" };
    if (!j.at("cycles").is_number_unsigned())
        throw error{ "key 'cycles' must have unsigned type" };

    j.at("fps").get_to(r.fps);
    j.at("cycles").get_to(r.cycles);
};

void from_json(nlohmann::json const& j, input_data & r)
{
    j.at("simulation").get_to(r.input_simulation);
    j.at("world").get_to(r.input_world);
}

int main(int, char** argv)
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

    auto const cwd = std::filesystem::path(argv[0]).parent_path();
    auto const input_path = cwd / "game_of_life_input.json";
    input_data input;
    try
    {
        std::ifstream input_json_ifstream{ input_path };
        if (!input_json_ifstream.is_open())
            throw error{ "can't open file" };

        auto const input_json = nlohmann::json::parse(input_json_ifstream);
        input = input_json.get<input_data>();
    }
    catch (std::exception const& e)
    {
        log::err(
            "Error while loading {}:"
            "\n    what(): {}",
            input_path.string(), e.what()
        );
    }

    linear_scene scene;
    scene.camera = &scene.push<camera_scene_node>();
    scene.push<skybox_scene_node>("/home/missed/code/gol/assets/skybox/skybox.model");
    auto & world_node = scene.push<world_scene_node>(input.input_world);

    GOL_SCOPE_EXIT
    {
        nlohmann::json j;
        try
        {
            to_json(j["result"], world_node.gol_world);
            std::ofstream out{ cwd / "game_of_life_output.json" };
            out << j.dump(/*indent*/4);
        }
        catch (std::exception const& e)
        {
            log::err(
                "Something bad happens...:"
                "\n    what(): {}",
                e.what()
            );
        }
    };

    ticker render_ticker{ ticker::duration(std::chrono::seconds(1)) / input.input_simulation.fps };
    ticker simulation_ticker{ std::chrono::milliseconds(10) }; // limit simulation logic update by 10ms for no reason

    render_ticker.frame();
    simulation_ticker.frame();

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

        if (simulation_ticker.frame())
            scene.simulate(simulation_ticker.delta());

        if (!render_ticker.frame())
            continue;

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
    }

    return EXIT_SUCCESS;
}
