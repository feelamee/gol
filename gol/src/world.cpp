#include <gol/world.hpp>

#include <engine/ranges.hpp>

namespace gol
{

coord coord::up()
{
    return { row - 1, col };
}

coord coord::down()
{
    return { row + 1, col };
}

coord coord::left()
{
    return { row, col - 1 };
}

coord coord::right()
{
    return { row, col + 1 };
}


world::world(
    i32 row_count, i32 column_count,
    cell c
)
    : row_count(row_count)
    , column_count(column_count)
{
    check_invariants(row_count, column_count);
    data.assign(sz(row_count * column_count), c);
}

world::world(
    i32 row_count, i32 column_count,
    std::initializer_list<cell> const& d
)
    : row_count(row_count)
    , column_count(column_count)
{
    check_invariants(row_count, column_count, i32(d.size()));
    data.assign(begin(d), end(d));
}

cell & world::get(coord c)
{
    return data[to_index(c)];
}

cell const& world::get(coord c) const
{
    return data[to_index(c)];
}

cell & world::get(i32 row, i32 col)
{
    return get({ row, col });
}

cell const& world::get(i32 row, i32 col) const
{
    return get({ row, col });
}

std::optional<cell> world::at(coord c) const
{
    return at(c.row, c.col);
}

std::optional<cell> world::at(i32 row, i32 col) const
{
    if (row < 0 || row >= row_count)
        return std::nullopt;
    if (col < 0 || col >= column_count)
        return std::nullopt;
    return get(row, col);
}

coord world::to_coord(sz i) const
{
    return { .row = i32(i) / column_count, .col = i32(i) % column_count };
}

sz world::to_index(coord c) const
{
    return sz(c.row * column_count + c.col);
}

void world::iterate(u32 steps)
{
    // TODO! optimize

    while (steps--)
    {
        auto const alive_neighbors_count = [w = *this](coord c)
        {
            i32 count = 0;

            count += w.at(c.up())    == cell::alive;
            count += w.at(c.down())  == cell::alive;
            count += w.at(c.left())  == cell::alive;
            count += w.at(c.right()) == cell::alive;

            count += w.at(c.up().left())    == cell::alive;
            count += w.at(c.up().right())   == cell::alive;
            count += w.at(c.down().left())  == cell::alive;
            count += w.at(c.down().right()) == cell::alive;

            return count;
        };

        for (auto [i, cell] : vs::enumerate(data))
        {
            i32 const count = alive_neighbors_count(to_coord(i));

            if (cell == cell::alive)
            {
                if (count != 2 && count != 3) cell = cell::dead;
            }
            else if (cell == cell::dead)
            {
                if (count == 3) cell = cell::alive;
            }
        }
    }
}

void world::clear()
{
    row_count = 0;
    column_count = 0;
    data.clear();
}

bool world::empty() const
{
    return row_count == 0 && column_count == 0 && data.empty();
}

void world::resize(i32 row_count, i32 column_count)
{
    check_invariants(row_count, column_count);
    this->row_count = row_count;
    this->column_count = column_count;
    data.resize(sz(row_count * column_count));
}

void world::check_invariants(i32 row_count, i32 column_count, std::optional<i32> data_size)
{
    if (
        row_count >= 0 && column_count >= 0 &&
        (!data_size || row_count * column_count == data_size)
    )
        return;

    if (data_size)
    {
        throw error(
            "Invalid row/column count: {} rows, {} columns, {} data size",
            row_count, column_count, *data_size
        );
    }
    else
    {
        throw error(
            "Invalid row/column count: {} rows, {} columns",
            row_count, column_count
        );
    }
}

}

#include <doctest.h>

TEST_CASE("world::ctors")
{
    using namespace gol;

    world w;
    REQUIRE(w.row_count == 0);
    REQUIRE(w.column_count == 0);
    REQUIRE(w.data.size() == 0);

    w = world{ 3, 4 };
    REQUIRE(w.row_count == 3);
    REQUIRE(w.column_count == 4);
    REQUIRE(w.data.size() == 12);
    REQUIRE(rng::all_of(w.data, [](cell c){ return c == cell::dead; }));

    CHECK_THROWS_AS(world(3, 4, { cell::dead }), world::error);
    CHECK_THROWS_AS(world(-3, 4), world::error);
}

TEST_CASE("world::getters")
{
    using namespace gol;
    constexpr auto D = cell::dead;
    constexpr auto A = cell::alive;
    world w{
        3, 3,
        {
            A, A, D,
            D, D, A,
            A, D, A,
        }
    };
    REQUIRE(w.get(0, 0) == A);
    REQUIRE(w.get(0, 1) == A);
    REQUIRE(w.get(0, 2) == D);

    REQUIRE(w.get(1, 0) == D);
    REQUIRE(w.get(1, 1) == D);
    REQUIRE(w.get(1, 2) == A);

    REQUIRE(w.get(2, 0) == A);
    REQUIRE(w.get(2, 1) == D);
    REQUIRE(w.get(2, 2) == A);

    REQUIRE(w.at(0, 0) == A);
    REQUIRE(w.at(0, 1) == A);
    REQUIRE(w.at(0, 2) == D);

    REQUIRE(w.at(1, 0) == D);
    REQUIRE(w.at(1, 1) == D);
    REQUIRE(w.at(1, 2) == A);

    REQUIRE(w.at(2, 0) == A);
    REQUIRE(w.at(2, 1) == D);
    REQUIRE(w.at(2, 2) == A);

    REQUIRE(w.at(12, 0)  == std::nullopt);
    REQUIRE(w.at(2, -1)  == std::nullopt);
    REQUIRE(w.at(-4, 42) == std::nullopt);
}

#ifndef DOCTEST_CONFIG_DISABLE
namespace gol
{

static std::ostream & operator<<(std::ostream & os, gol::world const & w)
{
    using gol::i32, gol::cell;

    os << '{' << '\n';
    for (i32 row = 0; row < w.row_count; ++row)
    {
        os << "    ";
        for (i32 col = 0; col < w.column_count; ++col)
        {
            cell const c = w.get(row, col);
            os << (c == cell::alive ? 'A' : 'D') << ' ';
        }
        os << '\n';
    }
    os << '}' << '\n';

    return os;
}

}
#endif

TEST_CASE("world::iterate::oscillator")
{
    using namespace gol;
    constexpr auto D = cell::dead;
    constexpr auto A = cell::alive;

    // Осциллятор "Мигалка"
    world const input{
        5, 5,
        {
            D, D, D, D, D,
            D, D, A, D, D,
            D, D, A, D, D,
            D, D, A, D, D,
            D, D, D, D, D,
        }
    };
    world const output{
        5, 5,
        {
            D, D, D, D, D,
            D, D, D, D, D,
            D, A, A, A, D,
            D, D, D, D, D,
            D, D, D, D, D,
        }
    };

    auto const iterated = [](world w, u32 steps = 1) { w.iterate(steps); return w; };
    REQUIRE_EQ(iterated(input),    output);
    REQUIRE_EQ(iterated(input, 2), input);
    REQUIRE_EQ(iterated(input, 3), output);
    REQUIRE_EQ(iterated(input, 4), input);
}

TEST_CASE("world::iterate::empty")
{
    using namespace gol;
    constexpr auto D = cell::dead;

    // Пустой мир
    world const input{
        3, 3,
        {
            D, D, D,
            D, D, D,
            D, D, D,
        }
    };
    world const output{
        3, 3,
        {
            D, D, D,
            D, D, D,
            D, D, D,
        }
    };

    auto const iterated = [](world w, u32 steps = 1) { w.iterate(steps); return w; };
    REQUIRE_EQ(iterated(input),    output);
    REQUIRE_EQ(iterated(input, 3), output);
}

TEST_CASE("world::iterate::block")
{
    using namespace gol;
    constexpr auto D = cell::dead;
    constexpr auto A = cell::alive;

    // Стабильная конфигурация "Блок"
    world const input{
        4, 4,
        {
            D, D, D, D,
            D, A, A, D,
            D, A, A, D,
            D, D, D, D,
        }
    };
    world const output{
        4, 4,
        {
            D, D, D, D,
            D, A, A, D,
            D, A, A, D,
            D, D, D, D,
        }
    };

    auto const iterated = [](world w, u32 steps = 1) { w.iterate(steps); return w; };
    REQUIRE_EQ(iterated(input),    output);
    REQUIRE_EQ(iterated(input, 3), output);
}

TEST_CASE("world::clear,empty")
{
    using namespace gol;

    world w;
    REQUIRE(w.empty());

    world w2;
    w2.clear();
    REQUIRE(w2 == world{});
    REQUIRE(w2.empty());

    using namespace gol;
    constexpr auto D = cell::dead;
    constexpr auto A = cell::alive;
    world w3{
        4, 4,
        {
            D, D, D, D,
            D, A, A, D,
            D, A, A, D,
            D, D, D, D,
        }
    };
    REQUIRE(!w3.empty());
    REQUIRE(w3 != w2);
    w3.clear();
    REQUIRE(w3.empty());
    REQUIRE(w3 == w2);
}

TEST_CASE("world::resize")
{
    using namespace gol;

    // dont provide any garantees about world values to simplify impl
    world w;
    w.resize(0, 0);
    REQUIRE(w == world{});

    CHECK_THROWS_AS(w.resize(3, -4), world::error);
    CHECK_THROWS_AS(w.resize(-3, 4), world::error);

    w.resize(1, 2);
    REQUIRE(w.row_count    == 1);
    REQUIRE(w.column_count == 2);
}
