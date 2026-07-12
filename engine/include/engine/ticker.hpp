#pragma once

#include <chrono>

namespace gol
{

// TODO! better name
struct ticker
{
    using clock = std::chrono::high_resolution_clock;
    using duration = clock::duration;
    using time_point = clock::time_point;

    ticker(std::optional<duration> min_delta = std::nullopt);

    void set_min_delta(std::optional<duration> d);

    // handy ticker, for use without clock, just like accumulator
    bool frame(duration delta);
    bool frame();

    duration delta() const;

    time_point last_frame_time;
    duration last_frame_delta;

    time_point last_time;
    duration last_delta;
    std::optional<duration> min_delta;
};

}
