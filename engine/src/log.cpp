#include <engine/log.hpp>

#include <SDL3/SDL_log.h>

namespace gt::log
{

static SDL_LogCategory to_sdl(category c)
{
    switch (c)
    {
        case category::gl: return SDL_LOG_CATEGORY_CUSTOM;
        case category::sdl: return SDL_LogCategory(SDL_LOG_CATEGORY_CUSTOM + 1);
        case category::app: return SDL_LogCategory(SDL_LOG_CATEGORY_CUSTOM + 2);
    }

    unreachable();
}

static SDL_LogPriority to_sdl(priority p)
{
    switch (p)
    {
        case priority::info: return SDL_LOG_PRIORITY_INFO;
        case priority::warn: return SDL_LOG_PRIORITY_WARN;
        case priority::err: return SDL_LOG_PRIORITY_ERROR;
    }

    unreachable();
}

void vlog(category c, priority p, std::string_view fmt, std::format_args args)
{
    SDL_LogMessage(to_sdl(c), to_sdl(p), "%s", std::vformat(fmt, args).c_str());
}

}
