#pragma once

#include <engine/error.hpp>

namespace gol
{

struct coord
{
    coord up();
    coord down();
    coord left();
    coord right();

    i32 row;
    i32 col;
};

enum struct cell : bool
{
    dead = false,
    alive = true,
};

struct world
{
    struct error : gol::error
    {
        using gol::error::error;
    };

    world() = default;

    world(
        i32 row_count, i32 column_count,
        cell c = cell::dead
    );

    world(
        i32 row_count, i32 column_count,
        rng::range auto&& d
    )
        : row_count(row_count)
        , column_count(column_count)
    {
        check_invariants(row_count, column_count, i32(size(d)));
        data.assign(begin(d), end(d));
    }

    world(
        i32 row_count, i32 column_count,
        std::initializer_list<cell> const& d
    );

    cell & get(coord c);
    cell const& get(coord c) const;
    cell & get(i32 row, i32 col);
    cell const& get(i32 row, i32 col) const;

    std::optional<cell> at(coord c) const;
    std::optional<cell> at(i32 row, i32 col) const;

    coord to_coord(sz i) const;
    sz to_index(coord c) const;

    void iterate(u32 steps = 1);

    void clear();
    bool empty() const;

    void resize(i32 row_count, i32 column_count);

    bool operator==(world const&) const = default;

    // TODO! optimize by space, pack 8 cell's into one byte
    std::vector<cell> data;
    i32 row_count{ 0 };
    i32 column_count{ 0 };

private:
    static void check_invariants(i32 row_count, i32 column_count, std::optional<i32> data_size = {});
};

}
