#pragma once

#include <engine/scene.hpp>
#include <engine/ticker.hpp>

#include <gol/world.hpp>

namespace gol
{

struct world_scene_node : scene_node
{
    world_scene_node();
    world_scene_node(world w);
    ~world_scene_node() override;

    world gol_world; // TODO! pimpl? to not include world.hpp
    mutable gl::buffer gpu_world;
    gl::buffer ebo;
    gl::vertex_array vao;
    gl::shader shader;

    constexpr inline static u32 infinite_iterations = u32(-1);
    u32 least_iterations = infinite_iterations;

    // ticker to limit iterations speed of gol_world
    ticker iteration_ticker;
    mutable bool gol_world_changed = true;

private:
    void do_handle_event(SDL_Event) override;
    void do_simulate(delta_type delta) override;
    void do_draw(draw_info const & di) const override;
};

}

