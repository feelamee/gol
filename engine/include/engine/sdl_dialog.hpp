#pragma once

#include <engine/context.hpp>
#include <engine/sdl.hpp>

namespace gol
{

struct select_file_dialog_params
{
    bool many = false;
};

// TODO! move impl to cpp somehow
template<typename Fn>
void select_file_dialog(Fn&& fn, select_file_dialog_params const & params = {})
{
    SDL_ShowOpenFileDialog(
        [](void* userdata, char const* const* files, int)
        {
            auto * fn = static_cast<Fn *>(userdata);
            assert(fn);
            GOL_SCOPE_EXIT { delete fn; };

            if (!files)
            {
                sdl::log_error();
                return;
            }

            char const * const * end = files;
            while (*end)
                 ++end;

            auto r = rng::subrange(files, end)
                   | vs::transform([](char const * s) { return std::string_view{ s }; });

           std::invoke(*fn, r);
        },
        new Fn(std::forward<Fn>(fn)),
        ctx().window,
        nullptr,
        0,
        nullptr,
        params.many
    );
}

}
