#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_mixer/SDL_mixer.h>

#include <vector>

#include "slider.h"
#include "Menu.h"
#include "TicTacToe.h"

enum class GameState {
    Menu,
    TIC_TAC_TOE
};

struct App {
    std::vector<MenuItem> menuItems;
    std::vector<SideButton> sideButtons;

    int hoveredIndex = -1;

    Menu menu;
    TicTacToe tic;
    GameState state = GameState::Menu;

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_AppResult quitState = SDL_APP_CONTINUE;

    float scalex = 1.0f;
    float scaley = 1.0f;
    float scale = 1.0f;
    int WIDTH = 0;
    int HEIGHT = 0;

    // -------- Audio --------
    MIX_Mixer* mixer = nullptr;
    MIX_Track* musicTrack = nullptr;
    bool musicPaused = false;
    float volume = 0.2f;   // 0.0 تا 1.0
    Slider volumeSlider;

    // -------- Text --------
    TTF_Font* font = nullptr;
    float mouseX = 0.0f;
    float mouseY = 0.0f;

    // -------- FPS --------
    Uint64 lastCounter = 0;
    int frameCount = 0;
    float fps = 0.0f;
    Slider fpsSlider;
    bool limitFPS = true;
    int targetFPS = 60;
    float targetFPSFloat = 75.0f;
    double FRAME_TIME = 1000.0 / targetFPS; // ms
    SDL_Texture* fpsTex = nullptr;
    SDL_FRect fpsRect{};
};
