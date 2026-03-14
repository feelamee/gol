#include <engine/sdl.hpp>

#include <engine/error.hpp>
#include <engine/log.hpp>

namespace gt::sdl
{

void log_error(std::source_location l)
{
    log::err(
        log::category::sdl, "{}:{}: {}",
        l.file_name(), l.line(), SDL_GetError()
    );
}

void throw_error(std::source_location l)
{
    // TODO! split logging functions to WHO format message and to WHO print it
    // use formating functions here
    throw error(
        "[ERROR][SDL] {}:{}: {}",
        l.file_name(), l.line(), SDL_GetError()
    );
}

bool set_attr(SDL_GLAttr attr, int value)
{
    bool r = SDL_GL_SetAttribute(attr, value);
    if (!r)
        sdl::log_error();

    return r;
}

bool get_attr(SDL_GLAttr attr, int * value)
{
    bool r = SDL_GL_GetAttribute(attr, value);
    if (!r)
        sdl::log_error();

    return r;
}

}
