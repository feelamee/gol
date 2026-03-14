#pragma once

#include <format>

namespace gt::log
{

enum class category
{
    gl,
    sdl,
    app,
};

enum class priority
{
    info,
    warn,
    err,
};

void vlog(category, priority, std::string_view fmt, std::format_args args);

template<typename... Args>
void log(category c, priority p, std::format_string<Args...> fmt, Args const&... args)
{
    return vlog(c, p, fmt.get(), std::make_format_args(args...));
}

template<typename... Args>
void log(std::format_string<Args...> fmt, Args const&... args)
{
    return log(category::app, priority::info, fmt, args...);
}

template<typename... Args>
void info(category c, std::format_string<Args...> fmt, Args const&... args)
{
    return log(c, priority::info, std::move(fmt), args...);
}

template<typename... Args>
void warn(category c, std::format_string<Args...> fmt, Args const&... args)
{
    return log(c, priority::warn, std::move(fmt), args...);
}

template<typename... Args>
void err(category c, std::format_string<Args...> fmt, Args const&... args)
{
    return log(c, priority::err, std::move(fmt), args...);
}

template<typename... Args>
void info(std::format_string<Args...> fmt, Args const&... args)
{
    return info(category::app, std::move(fmt), args...);
}

template<typename... Args>
void warn(std::format_string<Args...> fmt, Args const&... args)
{
    return log(category::app, priority::warn, std::move(fmt), args...);
}

template<typename... Args>
void err(std::format_string<Args...> fmt, Args const&... args)
{
    return log(category::app, priority::err, std::move(fmt), args...);
}

}
