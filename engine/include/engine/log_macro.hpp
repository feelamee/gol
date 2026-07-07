#pragma once

#ifndef NDEBUG
#include <engine/log.hpp>
#define GOL_LOG_DEBUG(...) ::gol::log::log("{}:{}: {}", __FILE__, __LINE__, std::format(__VA_ARGS__))
#else
#define GOL_LOG_DEBUG(...)
#endif
