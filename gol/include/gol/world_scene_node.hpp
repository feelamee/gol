#pragma once

#include <engine/scene.hpp>

#include <gol/world.hpp>

namespace gol
{

struct world_scene_node : scene_node
{
    world_scene_node();
    world_scene_node(world w);
    ~world_scene_node() override;

    void handle_event(SDL_Event) override;
    void simulate(float delta) override;
    void draw(draw_info const & di) const override;

    world gol_world; /// TODO! pimpl? to not include world.hpp
    gl::buffer gpu_world;
    gl::buffer ebo;
    gl::vertex_array vao;
    gl::shader shader;
};

}

