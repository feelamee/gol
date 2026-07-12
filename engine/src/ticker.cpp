#include <engine/ticker.hpp>

namespace gol
{

ticker::ticker(std::optional<duration> min_delta)
    : last_frame_time{}
    , last_frame_delta{ 0 }
    , last_time{ last_frame_time }
    , last_delta{ last_frame_delta }
    , min_delta(min_delta)
{
}

void ticker::set_min_delta(std::optional<duration> d)
{
    min_delta = d;
}

bool ticker::frame(duration delta)
{
    auto const prev_last_time = last_time;
    last_time = prev_last_time + delta;;
    last_delta += delta;

    if (min_delta && last_delta < *min_delta)
        return false;

    last_frame_time = last_time;
    last_frame_delta = last_delta;
    last_delta = duration{ 0 };
    return true;
}

bool ticker::frame()
{
    return frame(clock::now() - last_time);
}

ticker::duration ticker::delta() const
{
    return last_frame_delta;
}

}
