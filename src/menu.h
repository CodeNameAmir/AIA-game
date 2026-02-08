#pragma once

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>

enum class MenuItemType {
    TicTacToe,
    SeaBattle,
    Yahtzee,
    Memory,
    Chess,
    Snakes,
    DotsAndBoxes
};

struct UIItem {
    SDL_FRect baserect{};
    SDL_FRect rect{};
    SDL_Texture* icon = nullptr;

    bool hovered = false;
    float hoverScale = 1.1f;
    float spacex = 0.0f;
    float spacey = 0.0f;
    Uint8 normalAlpha = 180;
    Uint8 hoverAlpha = 255;
};

struct SideButton {
    UIItem ui;
};

struct MenuItem {
    MenuItemType type{};
    UIItem ui;

    SDL_Texture* label = nullptr;
    SDL_FRect labelRect{};
};

struct App;

struct Menu {
    App* app = nullptr;

    SDL_Texture* logoTex = nullptr;
    SDL_FRect logoRect{};
    SDL_FRect logobaseRect{};

    SDL_Texture* logobgTex = nullptr;
    SDL_FRect logobgRect{};
    SDL_FRect logobgbaseRect{};

    SDL_Texture* sidemenuitembgTex = nullptr;
    SDL_FRect sidemenuitembgRect{};
    SDL_FRect sidemenuitembgbaseRect{};
};

void Menu_Init(Menu& game, SDL_Renderer* renderer, App* app);
void Menu_Update(Menu& game);
void Menu_Render(Menu& game, SDL_Renderer* renderer);
void Menu_HandleEvent(Menu& game, SDL_Event* e);
void Menu_Quit(Menu& game);
