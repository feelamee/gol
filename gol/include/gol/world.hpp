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

struct packed_cells
{
    using storage_type = u8;
    constexpr inline static sz size = sizeof(storage_type) * 8;

    packed_cells();
    packed_cells(cell c);

    cell get(sz i) const;
    void set(sz i, cell c);
    void set(cell c);

    bool operator==(packed_cells const&) const = default;

    //   76 54 32 10
    // 0b00'00'00'00
    storage_type data; //< TODO! std::bitset?
};

struct world
{
    struct error : gol::error
    {
        using gol::error::error;
    };

    using storage_type = std::vector<packed_cells>;

    world() = default;

    world(
        i32 row_count, i32 column_count,
        cell c = cell::dead
    );

    world(
        i32 row_count, i32 column_count,
        rng::range auto const& d
    )
        : row_count(row_count)
        , column_count(column_count)
    {
        check_invariants(row_count, column_count, i32(size(d)));
        assign_to_storage(d, data);
        prev_data.resize(data.size());
    }

    world(
        i32 row_count, i32 column_count,
        std::initializer_list<cell> const& d
    );

    cell get(coord c) const;
    cell get(i32 row, i32 col) const;

    void set(coord c, cell v);
    void set(i32 row, i32 col, cell v);

    std::optional<cell> try_get(coord c) const;
    std::optional<cell> try_get(i32 row, i32 col) const;

    cell get(storage_type const &, coord c) const;
    cell get(storage_type const &, i32 row, i32 col) const;

    void set(storage_type &, coord c, cell v) const;
    void set(storage_type &, i32 row, i32 col, cell v) const;

    std::optional<cell> try_get(storage_type const &, coord c) const;
    std::optional<cell> try_get(storage_type const &, i32 row, i32 col) const;

    coord to_coord(sz i) const;
    sz to_index(coord c) const;

    void iterate(u32 steps = 1);

    void clear();
    bool empty() const;

    void resize(i32 row_count, i32 column_count, cell c = cell::dead);

    bool operator==(world const&) const;

    // TODO! better imple bitvector
    storage_type prev_data; //< previous world state. TODO! maybe allocate them as once memory chunk?
    storage_type data; //< actual/current world state
    i32 row_count{ 0 };
    i32 column_count{ 0 };

private:
    static void check_invariants(i32 row_count, i32 column_count, std::optional<i32> data_size = {});

    template<rng::range R>
        requires std::same_as<rng::range_value_t<R>, cell>
    static void assign_to_storage(R const& r, storage_type & s)
    {
        auto const size = rng::size(r);
        if (size == 0)
            return;

        s.resize((size - 1) / storage_type::value_type::size + 1);
        for (auto [i, c] : vs::enumerate(r))
        {
            sz const j = i / packed_cells::size;
            sz const k = i % packed_cells::size;
            s[j].set(k, c);
        }
    }
};

}
