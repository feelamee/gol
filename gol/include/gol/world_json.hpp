#pragma once

#include <nlohmann_json.hpp>

namespace gol
{
    struct world;

    // TODO! write doc
    void to_json(nlohmann::json & j, world const & w);
    void from_json(nlohmann::json const & j, world & w);
}
