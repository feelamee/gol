#include <gol/world_json.hpp>
#include <gol/world.hpp>

namespace gol
{

nlohmann::json to_json(world const & w)
{
    nlohmann::json j;
    j["width"] = w.column_count;
    j["height"] = w.row_count;

    for (i32 r = 0; r < w.row_count; ++r)
    {
        std::string s;
        s.resize(sz(w.column_count));
        for (i32 c = 0; c < w.column_count; ++c)
            s[sz(c)] = w.get(r, c) == cell::alive ? '*' : '.';

        j["cells"].push_back(std::move(s));
    }

    return j;
}

bool from_json(nlohmann::json const & j, world & w)
{
    // TODO!:
    // - return error description
    // - try to load even when incorrect format provided

    if (!j.is_object())
        return false;

    auto const& row_count    = j["width"];
    auto const& column_count = j["height"];
    if (!row_count.is_number())
        return false;
    if (!column_count.is_number())
        return false;

    w.clear();
    w.resize(row_count, column_count);

    auto const& cells = j["cells"];
    if (!cells.is_array())
        return false;
    if (std::cmp_not_equal(cells.size(), w.row_count))
        return false;

    for (auto const& [r, row] : vs::enumerate_with<i32>(cells))
    {
        if (!row.is_string())
            return false;

        auto const& rowstr = row.get_ref<nlohmann::json::string_t const&>();
        if (std::cmp_not_equal(rowstr.size(), w.column_count))
            return false;

        for (auto const& [c, cell] : vs::enumerate_with<i32>(rowstr))
        {
            switch (cell)
            {
                case '*': w.get(r, c) = cell::alive; break;
                case '.': w.get(r, c) = cell::dead; break;
                default: return false;
            }
        }
    }

    return true;
}

}

#include <doctest.h>

TEST_CASE("world::to_json,from_json")
{
    using namespace gol;

    world w{ 3, 3 };
    auto j = to_json(w);

    INFO(w);
    INFO(j.dump());

    REQUIRE(j.is_object());

    REQUIRE(j["width"].is_number());
    REQUIRE(j["width"] == w.row_count);

    REQUIRE(j["height"].is_number());
    REQUIRE(j["height"] == w.column_count);

    REQUIRE(j["cells"].is_array());
    REQUIRE(j["cells"].size() == w.row_count);

    for (auto const& [r, row] : vs::enumerate_with<i32>(j["cells"]))
    {
        REQUIRE(row.is_string());

        auto const& rowstr = row.get_ref<nlohmann::json::string_t const&>();
        REQUIRE(rowstr.size() == w.column_count);

        for (auto const& [c, cell] : vs::enumerate_with<i32>(rowstr))
        {
            switch (cell)
            {
                case '*': REQUIRE(w.get(r, c) == cell::alive); break;
                case '.': REQUIRE(w.get(r, c) == cell::dead); break;
            }
        }
    }

    world w2;
    REQUIRE(from_json(j, w2));
    REQUIRE(w2 == w);

    auto const j2 = nlohmann::json::parse(R"({ "width": 1, "height": 1, "cells": [ "." ] })");
    REQUIRE(from_json(j2, w));
    REQUIRE(w.row_count == 1);
    REQUIRE(w.column_count == 1);
    REQUIRE(w.get(0, 0) == cell::dead);

    REQUIRE_FALSE(from_json(R"({ "width": 1, "height": 1, "cells": [ 5 ] })", w));
    REQUIRE_FALSE(from_json(R"({ "width": 1, "height": 1 })", w));
    REQUIRE_FALSE(from_json(R"({ "width": 1, "cells": [ "*" ] })", w));
    REQUIRE_FALSE(from_json(R"({ "width": [], "height": 1, "cells": [ "*" ] })", w));
    REQUIRE_FALSE(from_json(R"({ "width": -1, "height": 1, "cells": [] })", w));
}
