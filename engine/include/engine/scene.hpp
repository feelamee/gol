#pragma once

#include <engine/math.hpp>
#include <engine/gl.hpp>
#include <engine/camera.hpp>

namespace gol
{

struct scene_node
{
    virtual ~scene_node() = default;

    virtual void handle_event(SDL_Event);
    virtual void simulate(float delta);

    struct draw_info
    {
        mat4 view;
        mat4 projection;
    };
    virtual void draw(draw_info const & di) const;

    mat4 model_mat() const;

    vec3 pos{ 0.0f, 0.0f, 0.0f };
    f32 scaling{ 1.0f };
    f32 rotation{ 0.0f };

    mutable mat4 transform; //< cached model_mat
    mutable bool dirty_transform = true;
};

struct model_scene_node : scene_node
{
    model_scene_node(gl::model m, gl::shader s);

    void draw(draw_info const & di) const override;

    gl::model model;
    gl::shader shader;
};

struct skybox_scene_node : scene_node
{
    skybox_scene_node(gl::model m, gl::shader s);

    void draw(draw_info const& di) const override;

    gl::model model;
    gl::shader shader;
};

struct linear_scene
{
    void handle_event(SDL_Event const & ev);
    void simulate(float delta);

    struct draw_info
    {
        mat4 projection;
    };
    void draw(draw_info const & di) const;

    template<typename T, typename... Args>
    void push(Args&&... args)
    {
        objects.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

    std::vector<std::unique_ptr<scene_node>> objects;
    camera cam;
};

}
