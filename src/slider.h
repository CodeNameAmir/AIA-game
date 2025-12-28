#pragma once
#include <SDL3/SDL.h>

struct Slider {
    SDL_FRect bar;
    SDL_FRect knob;
    float minValue;
    float maxValue;
    float* value;
    bool dragging = false;
};
