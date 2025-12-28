#include "main.h"
#include "TicTacToe.h"
#include "menu.h"
#include "App.h"
#include "slider.h"

// --------------------------------------------------
// Constants
// --------------------------------------------------
constexpr int WINDOW_WIDTH  = 1280;
constexpr int WINDOW_HEIGHT = 720;



// --------------------------------------------------
// App State
// --------------------------------------------------
struct Star {
    float x, y;
    float speed;
    // Uint8 alpha;
};
std::vector<Star> stars;


// struct App {
//     // Core
//     TicTacToe tic;
//     Menu menu;
//     GameState state = GameState::Menu;
//     SDL_Window*   window   = nullptr;
//     SDL_Renderer* renderer = nullptr;
//     SDL_AppResult quitState = SDL_APP_CONTINUE;
//     float scalex=1.0;
//     float scaley=1.0;
//     int WIDTH,HEIGHT;

//     // -------- Audio --------
//     MIX_Mixer* mixer = nullptr;
//     MIX_Track* musicTrack = nullptr;
//     bool musicPaused = false;
//     float volume = 0.0f;   // 0.0 تا 1.0
//     Slider volumeSlider;

//     // -------- Text --------
//     TTF_Font* font = nullptr;
//     //mouse position
//     float mouseX = 0.0f;
//     float mouseY = 0.0f;

//     // -------- Textures --------
    

    

//     // -------- FPS --------
//     Uint64 lastCounter = 0;
//     int frameCount = 0;
//     float fps = 0.0f;
//     Slider fpsSlider;
//     bool limitFPS = true;
//     int targetFPS = 60;
//     float targetFPSFloat = 75.0f;
//     double FRAME_TIME = 1000.0 / targetFPS; // ms
//     SDL_Texture* fpsTex = nullptr;
//     SDL_FRect fpsRect{};

    

// };


// --------------------------------------------------
// Utility
// --------------------------------------------------
SDL_AppResult Fail(const char* msg) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: %s", msg, SDL_GetError());
    return SDL_APP_FAILURE;
}
std::filesystem::path GetBasePath() {
#if __ANDROID__
    return "";
#else
    const char* base = SDL_GetBasePath();
    if (!base) return "";
    return std::filesystem::path(base);
#endif
}


// --------------------------------------------------
// FPS
// --------------------------------------------------
void UpdateFPSText(App* app) {
    if (app->fpsTex) {
        SDL_DestroyTexture(app->fpsTex);
        app->fpsTex = nullptr;
    }

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "FPS: %.1f", app->fps);

    SDL_Color white{255,255,255,255};
    SDL_Surface* surf = TTF_RenderText_Solid(app->font, buffer, SDL_strlen(buffer), white);

    if (!surf) return;

    app->fpsTex = SDL_CreateTextureFromSurface(app->renderer, surf);
    SDL_DestroySurface(surf);

    auto props = SDL_GetTextureProperties(app->fpsTex);
    app->fpsRect.w = (float)SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_WIDTH_NUMBER, 0);
    app->fpsRect.h = (float)SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_HEIGHT_NUMBER, 0);

    app->fpsRect.x = app->WIDTH - app->fpsRect.w - 20;
    app->fpsRect.y = 10;
}
//backgorund stars
void InitStars(int count, int w, int h) {
    stars.resize(count);
    for (auto& s : stars) {
        s.x = (float)(rand() % w);
        s.y = (float)(rand() % h);
        s.speed = 0.2f + (rand() % 10) * 0.05f;
    }
}
inline void UpdateStars(int w, int h,SDL_Renderer* r) {
    SDL_SetRenderDrawColor(r, 255, 255, 255, 60);
    for (auto& s : stars) {
        s.y += s.speed;
        if (s.y >= h) s.y = 0;
        SDL_RenderPoint(r, (int)s.x, (int)s.y);
    }
}




// --------------------------------------------------
// SDL Callbacks
// --------------------------------------------------

SDL_AppResult SDL_AppInit(void** state, int, char**) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) return Fail("SDL_Init");
    if (!TTF_Init()) return Fail("TTF_Init");
    if (!MIX_Init()) return Fail("MIX_Init");

    auto* app = new App();
    
    


    app->window = SDL_CreateWindow("AIA game",WINDOW_WIDTH, WINDOW_HEIGHT,SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    app->WIDTH=WINDOW_WIDTH;
    app->HEIGHT=WINDOW_HEIGHT;
    // بعد از SDL_CreateWindow و قبل از return
    SDL_DisplayID display = SDL_GetDisplayForWindow(app->window);
    const SDL_DisplayMode* current_mode = SDL_GetCurrentDisplayMode(display);
    int refresh_rate = (int)SDL_roundf(current_mode->refresh_rate);
    // Volume slider
    app->volumeSlider = {
        .bar = {50, 330, 200, 8},
        .knob = {50+200*app->volume, 324, 12, 20},
        .minValue = 0.0f,
        .maxValue = 1.0f,
        .value = &app->volume
    };
    
    // FPS slider
    float knobpersent=(app->targetFPSFloat-30)/(float(refresh_rate)-30);
    app->fpsSlider = {
    .bar = {300, 330, 150, 8},
    .knob = {300+150*knobpersent, 324, 12, 20},
    .minValue = 30.0f,
    .maxValue = float(refresh_rate),
    .value = &app->targetFPSFloat
};
    if (!app->window) return Fail("CreateWindow");

    app->renderer = SDL_CreateRenderer(app->window, nullptr);
    // SDL_SetRenderBlendMode(app->renderer, SDL_BLENDMODE_BLEND);
    if (!app->renderer) return Fail("CreateRenderer");

    // SDL_SetRenderVSync(app->renderer, -1);

    auto basePath = GetBasePath();
    //background star
    InitStars(120, app->WIDTH, app->HEIGHT);

    // Font
    app->font = TTF_OpenFont((basePath / "Inter-VariableFont.ttf").string().c_str(), 36);
    if (!app->font) return Fail("OpenFont");

    
    
    
    *state = app;
    //audio
    app->mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (!app->mixer) return Fail("Create mixer");

    app->musicTrack = MIX_CreateTrack(app->mixer);

    auto musicPath = basePath / "the_entertainer.ogg";
    auto music = MIX_LoadAudio(app->mixer, musicPath.string().c_str(), false);
    if (!music) return Fail("Load music");

    MIX_SetTrackAudio(app->musicTrack, music);
    MIX_SetTrackGain(app->musicTrack, app->volume);
    MIX_PlayTrack(app->musicTrack, 0);


    SDL_ShowWindow(app->window);

    SDL_Log("App initialized");
    SDL_ShowWindow(app->window);
    {
        int  bbwidth, bbheight;
        SDL_GetWindowSize(app->window, &app->WIDTH, &app->HEIGHT);
        SDL_GetWindowSizeInPixels(app->window, &bbwidth, &bbheight);
        SDL_Log("Window size: %ix%i", app->WIDTH, app->HEIGHT);
        SDL_Log("Backbuffer size: %ix%i", bbwidth, bbheight);
        if (app->WIDTH != bbwidth){
            SDL_Log("This is a highdpi environment.");
        }
    }
    Menu_Init(app->menu,app->renderer,app);
    TicTacToe_Init(app->tic,app->renderer);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* state, SDL_Event* e) {
    auto* app = static_cast<App*>(state);
    
    if (e->type == SDL_EVENT_QUIT)
        app->quitState = SDL_APP_SUCCESS;

    if (e->type == SDL_EVENT_WINDOW_RESIZED){
        UpdateFPSText(app);
        // int width, height;
        SDL_GetWindowSize(app->window, &app->WIDTH, &app->HEIGHT);
        app->scalex=(float)app->WIDTH/WINDOW_WIDTH;
        app->scaley=(float)app->HEIGHT/WINDOW_HEIGHT;
        app->scale = app->scalex>app->scaley ? app->scaley:app->scalex;
        SDL_Log("Window size: %ix%i", app->WIDTH, app->HEIGHT);
        // std::cout<<app->scalex<<"\t"<<app->scaley<<"\t\t"<<app->scale<<"\t"<<std::endl;
        InitStars(int(app->WIDTH*app->HEIGHT/7680), app->WIDTH, app->HEIGHT);
        // SDL_Log("tedat setare ha : %i",int(app->WIDTH*app->HEIGHT/7680));

    }
    if (e->type == SDL_EVENT_MOUSE_MOTION) {
        app->mouseX = e->motion.x;
        app->mouseY = e->motion.y;

        // SDL_Log("Mouse position: X=%.1f Y=%.1f", app->mouseX, app->mouseY);
    }
        

    if (e->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        float mx = e->button.x;
        float my = e->button.y;

        auto hit = [&](SDL_FRect& r) {
            return mx >= r.x && mx <= r.x + r.w &&
                my >= r.y && my <= r.y + r.h;
        };

        if (hit(app->volumeSlider.knob)) app->volumeSlider.dragging = true;
        if (hit(app->fpsSlider.knob)) app->fpsSlider.dragging = true;
    }
    if (e->type == SDL_EVENT_MOUSE_BUTTON_UP) {
        app->volumeSlider.dragging = false;
        app->fpsSlider.dragging = false;
    }
    if (e->type == SDL_EVENT_MOUSE_MOTION) {
        float mx = e->motion.x;
        app->targetFPS = (int)app->targetFPSFloat;
        auto updateSlider = [&](Slider& s) {
            if (!s.dragging) return;

            float left = s.bar.x;
            float right = s.bar.x + s.bar.w;

            mx = std::clamp(mx, left, right);
            s.knob.x = mx - s.knob.w / 2;

            float t = (mx - left) / s.bar.w;
            *s.value = s.minValue + t * (s.maxValue - s.minValue);
        };

        updateSlider(app->volumeSlider);
        updateSlider(app->fpsSlider);

        MIX_SetTrackGain(app->musicTrack, app->volume);
    }

    if (e->type == SDL_EVENT_KEY_DOWN) {
    // std::cout << char(e->key.key) << std::endl;
    // SDL_Keycode key = e->key.key;
    SDL_Log("pressed key : %s",SDL_GetKeyName(e->key.key));
    switch (e->key.key) {


    case SDLK_Q:
        app->quitState = SDL_APP_SUCCESS;
        break;

    case SDLK_L:
        app->limitFPS = !app->limitFPS;
        SDL_Log("FPS Limit: %s", app->limitFPS ? "ON" : "OFF");
        break;

    case SDLK_P:
        if (app->musicPaused) {
            MIX_ResumeTrack(app->musicTrack);
        } else {
            MIX_PauseTrack(app->musicTrack);
        }
        app->musicPaused = !app->musicPaused;
        break;

    case SDLK_UP:
        app->volume += 0.1f;
        if (app->volume > 1.0f) app->volume = 1.0f;
        MIX_SetTrackGain(app->musicTrack, app->volume);
        SDL_Log("Volume: %i", int(app->volume*100));
        break;

    case SDLK_DOWN:
        app->volume -= 0.1f;
        if (app->volume < 0.0f) app->volume = 0.0f;
        MIX_SetTrackGain(app->musicTrack, app->volume);
        SDL_Log("Volume: %i", int(app->volume*100));
        break;
    //switch 
    case SDLK_G:
        if (app->state == GameState::TIC_TAC_TOE){
        app->state = GameState::Menu;
        SDL_Log("Switched to Menu");
        }
        else if (app->state == GameState::Menu){
        app->state = GameState::TIC_TAC_TOE;
        SDL_Log("Switched to Tic Tac Toe");
        }
        
    }
}
    if (app->state == GameState::TIC_TAC_TOE){
        TicTacToe_HandleEvent(app->tic, e);
    }
    if (app->state == GameState::Menu){
        Menu_HandleEvent(app->menu, e);
    }


    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* state) {
    auto* app = static_cast<App*>(state);

    // FPS
    Uint64 frameStart = SDL_GetTicks();
    Uint64 now = SDL_GetPerformanceCounter();
    
    app->frameCount++;

    if (app->lastCounter == 0)
        app->lastCounter = now;

    float elapsed = (now - app->lastCounter) /
                    (float)SDL_GetPerformanceFrequency();

    if (elapsed >= 1.0f) {
        app->fps = app->frameCount / elapsed;
        app->frameCount = 0;
        app->lastCounter = now;
        UpdateFPSText(app);
        // SDL_Log("FPS: %.1f", app->fps);
    }
    
    // auto drawSlider = [&](Slider& s) {
    // SDL_SetRenderDrawColor(app->renderer, 120,120,120,255);
    // SDL_RenderFillRect(app->renderer, &s.bar);

    // SDL_SetRenderDrawColor(app->renderer, 255,255,255,255);
    // SDL_RenderFillRect(app->renderer, &s.knob);
    // };

    
    
    // Background color animation
    // float t = SDL_GetTicks() / 1000.0f;
    SDL_SetRenderDrawColor(app->renderer,14, 46, 92, 255);
    SDL_RenderClear(app->renderer);

    UpdateStars(app->WIDTH, app->HEIGHT,app->renderer);

    // SDL_SetRenderDrawColor(app->renderer,(Uint8)((std::sin(t) + 1) * 127),(Uint8)((std::sin(t * 0.5f) + 1) * 127),200,255);

    if (app->state == GameState::Menu) {
        Menu_Update(app->menu);
        Menu_Render(app->menu, app->renderer);
        // drawSlider(app->volumeSlider);
        // drawSlider(app->fpsSlider);
        
    }
    else if (app->state == GameState::TIC_TAC_TOE) {
        TicTacToe_Update(app->tic);
        TicTacToe_Render(app->tic, app->renderer);
    }
        SDL_RenderTexture(app->renderer, app->fpsTex, nullptr, &app->fpsRect);
    // SDL_FRect rect = { 0.0f, 0.0f, WINDOW_WIDTH*app->scale, WINDOW_HEIGHT*app->scale };
    //     SDL_RenderRect(app->renderer, &rect);
        SDL_RenderPresent(app->renderer);

    // ---------------- FPS CAP ----------------
    if (app->limitFPS) {
        static Uint64 lastFrame = 0;
        Uint64 now = SDL_GetTicks();

        Uint32 frameTime = 1000 / app->targetFPS;
        Uint32 delta = now - lastFrame;

        if (delta < frameTime) {
            SDL_Delay(frameTime - delta);
        }

        lastFrame = SDL_GetTicks();
    }


    return app->quitState;
}

void SDL_AppQuit(void* state, SDL_AppResult) {
    auto* app = static_cast<App*>(state);

    if (app->musicTrack) {
        MIX_StopTrack(app->musicTrack, 0);
    }

    if (app->mixer) {
        MIX_DestroyMixer(app->mixer);
    }

    SDL_DestroyTexture(app->fpsTex);
    

    TTF_CloseFont(app->font);
    TicTacToe_Quit(app->tic);
    Menu_Quit(app->menu);
    SDL_DestroyRenderer(app->renderer);
    SDL_DestroyWindow(app->window);

    delete app;

    MIX_Quit();
    TTF_Quit();
    SDL_Quit();
}

