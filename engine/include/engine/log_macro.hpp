#pragma once

#ifndef NDEBUG
#include <engine/log.hpp>
#define GT_LOG_DEBUG(...) ::gt::log::log("{}:{}: {}", __FILE__, __LINE__, std::format(__VA_ARGS__))
#else
#define GT_LOG_DEBUG(...)
#endif
