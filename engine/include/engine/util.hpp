#pragma once

#include <engine/fundamental.hpp>

#include <type_traits>
#include <string_view>
#include <source_location>

namespace gol
{

template<typename E>
    requires std::is_enum_v<E>
auto to_underlying(E e)
{
    using t = std::underlying_type_t<E>;
    return static_cast<t>(e);
}

[[noreturn]] void unimplemented(
    std::source_location = std::source_location::current()
);

[[noreturn]] inline void
unreachable()
{
    // Uses compiler specific extensions if possible.
    // Even if no extension is used, undefined behavior is still raised by
    // an empty function body and the noreturn attribute.
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
    __assume(false);
#else // GCC, Clang
    __builtin_unreachable();
#endif
}

// from https://en.cppreference.com/w/cpp/numeric/byteswap.html
template<std::integral T>
constexpr T byteswap(T value) noexcept
{
    static_assert(std::has_unique_object_representations_v<T>,
                  "T may not have padding bits");
    auto value_representation = std::bit_cast<std::array<byte, sizeof(T)>>(value);
    std::ranges::reverse(value_representation);
    return std::bit_cast<T>(value_representation);
}

template<typename T>
void hash_combine(sz & seed, T const& v)
{
    using std::hash;
    seed ^= hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

} // namespace gol

namespace gol::detail
{

enum class scope_exit_type
{
    exit,    // invoke always
    success, // invoke only if no exception was thrown
    fail,    // invoke only if exception was thrown
};

template<typename Fn, scope_exit_type Type>
    requires (Type == scope_exit_type::success) || std::is_nothrow_invocable_v<Fn>
class scope_exit_impl
{
public:
    scope_exit_impl(Fn fn) noexcept(std::is_nothrow_move_constructible_v<Fn>)
        : fn{ std::move(fn) }
        , exceptions_count{ std::uncaught_exceptions() }
        , released{ false }
    {
    }

    ~scope_exit_impl() noexcept(Type != scope_exit_type::success)
    {
        if (released)
            return;

        if constexpr (Type == scope_exit_type::exit)
            fn();
        else if constexpr (Type == scope_exit_type::success)
        {
            if (exceptions_count == std::uncaught_exceptions())
                fn();
        }
        else if constexpr (Type == scope_exit_type::fail)
        {
            if (exceptions_count < std::uncaught_exceptions())
                fn();
        }
    }

    scope_exit_impl(scope_exit_impl const&) = delete;
    scope_exit_impl& operator=(scope_exit_impl const&) = delete;

    scope_exit_impl(scope_exit_impl && o) noexcept(std::is_nothrow_move_constructible_v<Fn>)
    {
        released = std::exchange(o.released, true);

        if (!released)
        {
            fn = std::move(o.fn);
            exceptions_count = o.exceptions_count;
        }
    }

    scope_exit_impl& operator=(scope_exit_impl o) noexcept(std::is_nothrow_swappable_v<scope_exit_impl>)
    {
        swap(*this, o);
        return *this;
    }

    void swap(scope_exit_impl& o) noexcept(std::is_nothrow_swappable_v<Fn>)
    {
        using std::swap;
        swap(fn, o.fn);
        swap(exceptions_count, o.exceptions_count);
        swap(released, o.released);
    }

private:
    [[no_unique_address]] Fn fn;
    int exceptions_count;
    bool released;
};

} // namespace gol::detail

namespace gol
{
    template<typename Fn>
    using scope_exit = detail::scope_exit_impl<Fn, detail::scope_exit_type::exit>;

    template<typename Fn>
    using scope_success = detail::scope_exit_impl<Fn, detail::scope_exit_type::success>;

    template<typename Fn>
    using scope_fail = detail::scope_exit_impl<Fn, detail::scope_exit_type::fail>;
}

// TODO! remove or move to separate header
#define GOL_DETAIL_CONCAT(a, b) a ## b
#define GOL_DETAIL_UNIQUE_ID(l) GOL_DETAIL_CONCAT(GOL_DETAIL_UNIQUE_ID_, l)

#define GOL_SCOPE_EXIT [[maybe_unused]] ::gol::scope_exit GOL_DETAIL_UNIQUE_ID(__LINE__) = [&]() noexcept
#define GOL_SCOPE_SUCCESS [[maybe_unused]] ::gol::scope_success GOL_DETAIL_UNIQUE_ID(__LINE__) = [&]()
#define GOL_SCOPE_FAIL [[maybe_unused]] ::gol::scope_fail GOL_DETAIL_UNIQUE_ID(__LINE__) = [&]() noexcept
