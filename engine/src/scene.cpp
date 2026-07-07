#include <engine/scene.hpp>

namespace gt
{

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
    }

    return transform;
}


model_scene_node::model_scene_node(gl::model m, gl::shader s)
    : model(std::move(m))
    , shader(s)
{}

void model_scene_node::draw(draw_info const & di) const
{
    using gt::gl::bind;
    bind(shader);
    bind(3, model_mat());
    bind(4, di.view);
    bind(5, di.projection);
    bind(6, model);

    glDrawElements(GL_TRIANGLES, GLsizei(model.indices_count), GL_UNSIGNED_INT, nullptr);
}


skybox_scene_node::skybox_scene_node(gl::model m, gl::shader s)
    : model(std::move(m))
    , shader(s)
{
}

void skybox_scene_node::draw(draw_info const& di) const
{
    using gl::bind;
    glDepthFunc(GL_LEQUAL);
    bind(shader);

    using gt::gl::bind;
    bind(1, di.view);
    bind(2, di.projection);
    bind(3, model);

    glDrawElements(GL_TRIANGLES, GLsizei(model.indices_count), GL_UNSIGNED_INT, nullptr);
    glDepthFunc(GL_LESS);
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
