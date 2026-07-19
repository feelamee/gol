#include <gol/world_json.hpp>
#include <gol/world.hpp>

namespace gol
{

void to_json(nlohmann::json & j, world const & w)
{
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
}

void from_json(nlohmann::json const & j, world & w)
{
    i32 const width = j.at("width");
    i32 const height = j.at("height");

    w.clear();
    w.resize(height, width);

    auto const& cells = j.find("cells");
    if (cells == j.end())
        throw world::error{ "json must have 'cells' key" };
    if (!cells.value().is_array())
        throw world::error{ "'cells' key must be array" };
    if (std::cmp_not_equal(cells.value().size(), w.row_count))
        throw world::error{ "'cells' array must be of size 'height' {} {}", cells.value().size(), w.row_count };

    for (auto const& [r, row] : vs::enumerate_with<i32>(cells.value()))
    {
        if (!row.is_string())
            throw world::error{ "'cells' must be array of strings" };

        auto const& rowstr = row.get_ref<nlohmann::json::string_t const&>();
        if (std::cmp_not_equal(rowstr.size(), w.column_count))
            throw world::error{ "each element of 'cells' must be of size 'width'" };

        for (auto const& [c, cell] : vs::enumerate_with<i32>(rowstr))
        {
            switch (cell)
            {
                case '*': w.set(r, c, cell::alive); break;
                case '.': w.set(r, c, cell::dead); break;
                default: throw world::error{ "each element of 'cells' must be string of '*' or '.' characters" };
            }
        }
    }
}

}

#include <doctest.h>

TEST_CASE("world::to_json,from_json")
{
    using namespace gol;

    world w{ 3, 3 };
    nlohmann::json j;
    REQUIRE_NOTHROW(to_json(j, w));

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
    REQUIRE_NOTHROW(from_json(j, w2));
    REQUIRE(w2 == w);

    auto const j2 = nlohmann::json::parse(R"({ "width": 1, "height": 1, "cells": [ "." ] })");
    REQUIRE_NOTHROW(from_json(j2, w));
    REQUIRE(w.row_count == 1);
    REQUIRE(w.column_count == 1);
    REQUIRE(w.get(0, 0) == cell::dead);

    CHECK_THROWS(from_json(R"({ "width": 1, "height": 1, "cells": [ 5 ] })", w));
    CHECK_THROWS(from_json(R"({ "width": 1, "height": 1 })", w));
    CHECK_THROWS(from_json(R"({ "width": 1, "cells": [ "*" ] })", w));
    CHECK_THROWS(from_json(R"({ "width": [], "height": 1, "cells": [ "*" ] })", w));
    CHECK_THROWS(from_json(R"({ "width": -1, "height": 1, "cells": [] })", w));
}
