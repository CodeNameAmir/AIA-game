#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>

#include <filesystem>
#include <cmath>
#include <iostream>
#include <thread>
// --------------------------------------------------
// Constants
// --------------------------------------------------
constexpr int WINDOW_WIDTH  = 500;
constexpr int WINDOW_HEIGHT = 400;

// --------------------------------------------------
// App State
// --------------------------------------------------
struct App {
    // Core
    SDL_Window*   window   = nullptr;
    SDL_Renderer* renderer = nullptr;

    // Textures
    SDL_Texture* backgroundTex = nullptr;
    SDL_Texture* messageTex    = nullptr;
    SDL_Texture* fpsTex        = nullptr;

    //audio
    SDL_AudioDeviceID audioDevice;
    MIX_Track* track;

    // Text rendering
    TTF_Font* font = nullptr;
    SDL_FRect messageRect{};
    SDL_FRect fpsRect{};
    SDL_FRect imageRect{0, 0, 300, 300};

    // Audio
    MIX_Track* musicTrack = nullptr;

    // FPS
    Uint64 lastCounter = 0;
    int frameCount = 0;
    float fps = 0.0f;

    SDL_AppResult quitState = SDL_APP_CONTINUE;
    // Image toggle
    SDL_Texture* legoTex = nullptr;
    SDL_FRect legoRect{100, 100, 128, 128};
    bool showLego = false;

};

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
    SDL_Surface* surf = TTF_RenderText_Solid(
        app->font, buffer, SDL_strlen(buffer), white
    );

    if (!surf) return;

    app->fpsTex = SDL_CreateTextureFromSurface(app->renderer, surf);
    SDL_DestroySurface(surf);

    auto props = SDL_GetTextureProperties(app->fpsTex);
    app->fpsRect.w = (float)SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_WIDTH_NUMBER, 0);
    app->fpsRect.h = (float)SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_HEIGHT_NUMBER, 0);

    int w, h;
    SDL_GetWindowSize(app->window, &w, &h);
    app->fpsRect.x = w - app->fpsRect.w - 20;
    app->fpsRect.y = 10;
}

// --------------------------------------------------
// SDL Callbacks
// --------------------------------------------------
SDL_AppResult SDL_Fail(){
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return SDL_APP_FAILURE;
}
SDL_AppResult SDL_AppInit(void** state, int, char**) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) return Fail("SDL_Init");
    if (!TTF_Init()) return Fail("TTF_Init");
    if (!MIX_Init()) return Fail("MIX_Init");

    auto* app = new App();

    app->window = SDL_CreateWindow(
        "SDL3 Clean Sample",
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
    );
    if (!app->window) return Fail("CreateWindow");

    app->renderer = SDL_CreateRenderer(app->window, nullptr);
    if (!app->renderer) return Fail("CreateRenderer");

    SDL_SetRenderVSync(app->renderer, -1);

    auto basePath = GetBasePath();

    // Font
    app->font = TTF_OpenFont((basePath / "Inter-VariableFont.ttf").string().c_str(), 36);
    if (!app->font) return Fail("OpenFont");

    // Message
    SDL_Surface* msgSurf = TTF_RenderText_Solid(
        app->font, "Hello SDL3!", 10, {255,255,255,255}
    );
    app->messageTex = SDL_CreateTextureFromSurface(app->renderer, msgSurf);
    SDL_DestroySurface(msgSurf);

    auto props = SDL_GetTextureProperties(app->messageTex);
    app->messageRect = {
        20, 20,
        (float)SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_WIDTH_NUMBER, 0),
        (float)SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_HEIGHT_NUMBER, 0)
    };

    // Image
    SDL_Surface* img = IMG_Load((basePath / "gs_tiger.svg").string().c_str());
    app->backgroundTex = SDL_CreateTextureFromSurface(app->renderer, img);
    SDL_DestroySurface(img);
    SDL_Surface* legoSurf = IMG_Load((basePath / "lego.png").string().c_str());
    if (!legoSurf) return Fail("Load lego.png");

    app->legoTex = SDL_CreateTextureFromSurface(app->renderer, legoSurf);
    SDL_DestroySurface(legoSurf);

    *state = app;
    // init SDL Mixer
    MIX_Mixer* mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if (mixer == nullptr) {
        return SDL_Fail();
    }
    
    auto mixerTrack = MIX_CreateTrack(mixer);

    // load the music
    auto musicPath = basePath / "the_entertainer.ogg";
    auto music = MIX_LoadAudio(mixer,musicPath.string().c_str(),false);
    if (not music) {
        return SDL_Fail();
    }

    // play the music (does not loop)
    MIX_SetTrackAudio(mixerTrack, music);
    MIX_PlayTrack(mixerTrack, 0);
    SDL_ShowWindow(app->window);

    SDL_Log("App initialized");
    SDL_ShowWindow(app->window);
    {
        int width, height, bbwidth, bbheight;
        SDL_GetWindowSize(app->window, &width, &height);
        SDL_GetWindowSizeInPixels(app->window, &bbwidth, &bbheight);
        SDL_Log("Window size: %ix%i", width, height);
        SDL_Log("Backbuffer size: %ix%i", bbwidth, bbheight);
        if (width != bbwidth){
            SDL_Log("This is a highdpi environment.");
        }
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* state, SDL_Event* e) {
    auto* app = static_cast<App*>(state);

    if (e->type == SDL_EVENT_QUIT)
        app->quitState = SDL_APP_SUCCESS;

    if (e->type == SDL_EVENT_WINDOW_RESIZED)
        UpdateFPSText(app);

    if (e->type == SDL_EVENT_KEY_DOWN) {
        if (e->key.key == SDLK_F) {
            app->showLego = !app->showLego;
        }
}


    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* state) {
    auto* app = static_cast<App*>(state);

    // FPS
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
        SDL_Log("FPS: %.1f", app->fps);
    }

    // Background color animation
    float t = SDL_GetTicks() / 1000.0f;
    SDL_SetRenderDrawColor(app->renderer,0,0,0,255);
    // SDL_SetRenderDrawColor(app->renderer,(Uint8)((std::sin(t) + 1) * 127),(Uint8)((std::sin(t * 0.5f) + 1) * 127),200,255);

    SDL_RenderClear(app->renderer);
    SDL_RenderTexture(app->renderer, app->backgroundTex, nullptr, &app->imageRect);
    SDL_RenderTexture(app->renderer, app->messageTex, nullptr, &app->messageRect);
    
    if (app->fpsTex)
        SDL_RenderTexture(app->renderer, app->fpsTex, nullptr, &app->fpsRect);

    if (app->showLego && app->legoTex) {
    SDL_RenderTexture(app->renderer, app->legoTex, nullptr, &app->legoRect);
        }

    SDL_RenderPresent(app->renderer);
    
    return app->quitState;
}

void SDL_AppQuit(void* state, SDL_AppResult) {
    auto* app = static_cast<App*>(state);
    MIX_StopTrack(app->track, MIX_TrackMSToFrames(app->track, 250));
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    SDL_DestroyTexture(app->backgroundTex);
    SDL_DestroyTexture(app->messageTex);
    SDL_DestroyTexture(app->fpsTex);

    TTF_CloseFont(app->font);
    SDL_DestroyTexture(app->legoTex);

    SDL_DestroyRenderer(app->renderer);
    SDL_DestroyWindow(app->window);

    delete app;

    MIX_Quit();
    TTF_Quit();
    SDL_Quit();
}
