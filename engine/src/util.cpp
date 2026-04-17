#include <engine/util.hpp>

#include <SDL3/SDL_log.h>

namespace gt
{

[[noreturn]] void unimplemented(std::source_location l)
{
    SDL_LogCritical(
        SDL_LOG_CATEGORY_ASSERT,
        "%s:%d:%d: %s: unimplemented...\n",
        l.file_name(), l.line(), l.column(), l.function_name()
    );

    std::abort();
}

}
