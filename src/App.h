#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_mixer/SDL_mixer.h>

#include <vector>
#include "slider.h"
#include "Menu.h"
#include "TicTacToe.h"
// struct Slider;
// struct Menu;
// struct TicTacToe;
void ApplyUIScale(float scale,SDL_FRect& item,const SDL_FRect& itembase) ;
SDL_Texture* LoadIcon(SDL_Renderer* r, const std::string& path) ;
void UpdateHover(UIItem& item, float mx, float my) ;
bool PointInRect(float mx, float my, const SDL_FRect& r) ;
void SidePanel_Render(App* app, SDL_Renderer* r) ;
void RendersidepanelItem(SDL_Renderer* r, UIItem& item, float uiScale,App* app,Menu& game) ;
void InitSidePanel(App* app);
enum class GameState {
    Menu,
    TIC_TAC_TOE
};

struct App {
    std::vector<MenuItem> menuItems;
    std::vector<SideButton> sideButtons;

    int hoveredIndex = -1;
    // Core
    Menu menu;
    TicTacToe tic;
    GameState state = GameState::Menu;
    SDL_Window*   window   = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_AppResult quitState = SDL_APP_CONTINUE;
    float scalex=1.0;
    float scaley=1.0;
    float scale=1.0;
    int WIDTH,HEIGHT;

    // -------- Audio --------
    MIX_Mixer* mixer = nullptr;
    MIX_Track* musicTrack = nullptr;
    bool musicPaused = false;
    float volume = 0.2f;   // 0.0 تا 1.0
    Slider volumeSlider;

    // -------- Text --------
    TTF_Font* font = nullptr;
    //mouse position
    float mouseX = 0.0f;
    float mouseY = 0.0f;

    // -------- Textures --------
    

    

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