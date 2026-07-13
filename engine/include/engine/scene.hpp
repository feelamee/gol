#pragma once

#include <engine/math.hpp>
#include <engine/gl.hpp>

#include <chrono>

namespace gol
{

struct scene_node
{
    virtual ~scene_node() = default;

    void handle_event(SDL_Event);

    using delta_type = std::chrono::nanoseconds;
    void simulate(delta_type delta);

    struct draw_info
    {
        mat4 view;
        mat4 projection;
    };
    void draw(draw_info const & di) const;


    mat4 model_mat() const;

    // TODO! this is weird and unusable. Maybe get rid of caching transfrom at all?
    vec3 get_position() const;
    void set_position(vec3 p);


    // enable/disable handle_event and simulate
    void enable(bool e);
    bool enabled() const;

    // enable/disable draw
    void visible(bool v);
    bool visible() const;

private: // TODO! maybe protected?
    virtual void do_handle_event(SDL_Event);
    virtual void do_simulate(delta_type delta);
    virtual void do_draw(draw_info const & di) const;

private:
    vec3 position{ 0.0f, 0.0f, 0.0f };
    f32 scaling{ 1.0f };
    f32 rotation{ 0.0f };

    mutable mat4 transform; //< cached model_mat
    mutable bool dirty_transform = true;

    // TODO! replace with bitflags?
    bool is_enabled = true;
    bool is_visible = true;
};

struct model_scene_node : scene_node
{
    model_scene_node();
    model_scene_node(std::filesystem::path const& model_path);

    bool set_model_path(std::filesystem::path const& p);

    gl::model model;
    gl::shader shader;

private:
    void do_draw(draw_info const & di) const override;
};

struct skybox_scene_node : scene_node
{
    skybox_scene_node();
    skybox_scene_node(std::filesystem::path const& model_path);
    ~skybox_scene_node() override;

    bool set_model_path(std::filesystem::path const& model_path);

    gl::model model;
    gl::shader shader;

private:
    void do_draw(draw_info const& di) const override;
};

struct camera_scene_node : scene_node
{
    camera_scene_node();
    camera_scene_node(vec3 const& pos, vec3 const& dir, vec3 const& up);

    mat4 view() const;

private:
    void do_handle_event(SDL_Event ev) override;
    void do_simulate(delta_type delta) override;

private:
    void update_vectors();

private:
    vec3 init_position;
    vec3 init_direction;
    f32 speed = 5.0f; // per second
    vec3 world_up{};

    vec3 direction{};

    vec3 right{};
    vec3 up{};

    bool is_w{ false };
    bool is_s{ false };
    bool is_a{ false };
    bool is_d{ false };
    bool is_lshift{ false };
    bool is_mouse_middle_button{ false };

    f32 init_pitch{ 0 };
    f32 init_yaw{ -90 };

    f32 pitch{ init_pitch };
    f32 yaw{ init_yaw };
};

struct linear_scene : scene_node
{
    template<typename T, typename... Args>
    T & push(Args&&... args)
    {
        objects.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
        return static_cast<T &>(*objects.back());
    }

    std::vector<std::unique_ptr<scene_node>> objects;
    camera_scene_node * camera = nullptr;

private:
    void do_handle_event(SDL_Event ev) override;
    void do_simulate(delta_type delta) override;
    void do_draw(draw_info const & di) const override;
};

}
