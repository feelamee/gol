#pragma once

#include <nlohmann_json.hpp>

namespace gol
{
    struct world;

    // TODO! write doc
    nlohmann::json to_json(world const & w);
    bool from_json(nlohmann::json const & j, world & w);
}
