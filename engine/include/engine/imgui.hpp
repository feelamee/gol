#pragma once

union SDL_Event;

namespace gol::im
{

// return true if imgui captured this event
bool handle_event(SDL_Event const&);
void frame();
void render();

}

