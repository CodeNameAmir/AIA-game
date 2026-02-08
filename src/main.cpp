#include "main.h"
#include "TicTacToe.h"
#include "menu.h"
#include "App.h"
#include "slider.h"

namespace {
constexpr int kWindowWidth = 1280;
constexpr int kWindowHeight = 720;
constexpr int kStarCountBaseArea = 7680;
constexpr int kStarCountDefault = 120;
constexpr int kFpsMin = 30;
constexpr int kFpsLabelOffsetX = 20;
constexpr int kFpsLabelOffsetY = 10;

struct Star {
    float x = 0.0f;
    float y = 0.0f;
    float speed = 0.0f;
};

std::vector<Star> gStars;

SDL_AppResult Fail(const char* msg) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: %s", msg, SDL_GetError());
    return SDL_APP_FAILURE;
}

std::filesystem::path GetBasePath() {
#if __ANDROID__
    return "";
#else
    const char* base = SDL_GetBasePath();
    if (!base) {
        return "";
    }
    return std::filesystem::path(base);
#endif
}

void UpdateFPSText(App* app) {
    if (app->fpsTex) {
        SDL_DestroyTexture(app->fpsTex);
        app->fpsTex = nullptr;
    }

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "FPS: %.1f", app->fps);

    SDL_Color white{255, 255, 255, 255};
    SDL_Surface* surf = TTF_RenderText_Solid(app->font, buffer, SDL_strlen(buffer), white);
    if (!surf) {
        return;
    }

    app->fpsTex = SDL_CreateTextureFromSurface(app->renderer, surf);
    SDL_DestroySurface(surf);
    if (!app->fpsTex) {
        return;
    }

    auto props = SDL_GetTextureProperties(app->fpsTex);
    app->fpsRect.w = static_cast<float>(SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_WIDTH_NUMBER, 0));
    app->fpsRect.h = static_cast<float>(SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_HEIGHT_NUMBER, 0));
    app->fpsRect.x = app->WIDTH - app->fpsRect.w - kFpsLabelOffsetX;
    app->fpsRect.y = kFpsLabelOffsetY;
}

void InitStars(int count, int w, int h) {
    gStars.resize(count);
    for (auto& star : gStars) {
        star.x = static_cast<float>(rand() % w);
        star.y = static_cast<float>(rand() % h);
        star.speed = 0.2f + (rand() % 10) * 0.05f;
    }
}

void UpdateStars(int w, int h, SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 60);
    for (auto& star : gStars) {
        star.y += star.speed;
        if (star.y >= h) {
            star.y = 0;
        }
        SDL_RenderPoint(renderer, static_cast<int>(star.x), static_cast<int>(star.y));
    }
}

void InitializeSliders(App* app, int refreshRate) {
    app->volumeSlider = {
        .bar = {50, 330, 200, 8},
        .knob = {50 + 200 * app->volume, 324, 12, 20},
        .minValue = 0.0f,
        .maxValue = 1.0f,
        .value = &app->volume
    };

    const float knobPercent = (app->targetFPSFloat - kFpsMin) /
                              (static_cast<float>(refreshRate) - kFpsMin);
    app->fpsSlider = {
        .bar = {300, 330, 150, 8},
        .knob = {300 + 150 * knobPercent, 324, 12, 20},
        .minValue = static_cast<float>(kFpsMin),
        .maxValue = static_cast<float>(refreshRate),
        .value = &app->targetFPSFloat
    };
}

void UpdateScale(App* app) {
    SDL_GetWindowSize(app->window, &app->WIDTH, &app->HEIGHT);
    app->scalex = static_cast<float>(app->WIDTH) / kWindowWidth;
    app->scaley = static_cast<float>(app->HEIGHT) / kWindowHeight;
    app->scale = app->scalex > app->scaley ? app->scaley : app->scalex;
}

void HandleSliderDrag(App* app, float mouseX) {
    app->targetFPS = static_cast<int>(app->targetFPSFloat);

    auto updateSlider = [&](Slider& slider) {
        if (!slider.dragging) {
            return;
        }

        const float left = slider.bar.x;
        const float right = slider.bar.x + slider.bar.w;
        const float clampedX = std::clamp(mouseX, left, right);
        slider.knob.x = clampedX - slider.knob.w / 2;

        const float t = (clampedX - left) / slider.bar.w;
        *slider.value = slider.minValue + t * (slider.maxValue - slider.minValue);
    };

    updateSlider(app->volumeSlider);
    updateSlider(app->fpsSlider);

    MIX_SetTrackGain(app->musicTrack, app->volume);
}

void HandleKeyDown(App* app, SDL_Keycode key) {
    SDL_Log("pressed key : %s", SDL_GetKeyName(key));

    switch (key) {
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
            app->volume = std::min(app->volume + 0.1f, 1.0f);
            MIX_SetTrackGain(app->musicTrack, app->volume);
            SDL_Log("Volume: %i", static_cast<int>(app->volume * 100));
            break;
        case SDLK_DOWN:
            app->volume = std::max(app->volume - 0.1f, 0.0f);
            MIX_SetTrackGain(app->musicTrack, app->volume);
            SDL_Log("Volume: %i", static_cast<int>(app->volume * 100));
            break;
        case SDLK_G:
            if (app->state == GameState::TIC_TAC_TOE) {
                app->state = GameState::Menu;
                SDL_Log("Switched to Menu");
            } else if (app->state == GameState::Menu) {
                app->state = GameState::TIC_TAC_TOE;
                SDL_Log("Switched to Tic Tac Toe");
            }
            break;
        default:
            break;
    }
}

} // namespace

SDL_AppResult SDL_AppInit(void** state, int, char**) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        return Fail("SDL_Init");
    }
    if (!TTF_Init()) {
        return Fail("TTF_Init");
    }
    if (!MIX_Init()) {
        return Fail("MIX_Init");
    }

    auto* app = new App();

    app->window = SDL_CreateWindow("AIA game", kWindowWidth, kWindowHeight,
                                   SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!app->window) {
        delete app;
        return Fail("CreateWindow");
    }

    app->WIDTH = kWindowWidth;
    app->HEIGHT = kWindowHeight;

    SDL_DisplayID display = SDL_GetDisplayForWindow(app->window);
    const SDL_DisplayMode* currentMode = SDL_GetCurrentDisplayMode(display);
    const int refreshRate = static_cast<int>(SDL_roundf(currentMode->refresh_rate));

    InitializeSliders(app, refreshRate);

    app->renderer = SDL_CreateRenderer(app->window, nullptr);
    if (!app->renderer) {
        delete app;
        return Fail("CreateRenderer");
    }

    auto basePath = GetBasePath();

    InitStars(kStarCountDefault, app->WIDTH, app->HEIGHT);

    app->font = TTF_OpenFont((basePath / "Inter-VariableFont.ttf").string().c_str(), 36);
    if (!app->font) {
        delete app;
        return Fail("OpenFont");
    }

    *state = app;

    app->mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (!app->mixer) {
        return Fail("Create mixer");
    }

    app->musicTrack = MIX_CreateTrack(app->mixer);
    auto musicPath = basePath / "the_entertainer.ogg";
    auto music = MIX_LoadAudio(app->mixer, musicPath.string().c_str(), false);
    if (!music) {
        return Fail("Load music");
    }

    MIX_SetTrackAudio(app->musicTrack, music);
    MIX_SetTrackGain(app->musicTrack, app->volume);
    MIX_PlayTrack(app->musicTrack, 0);

    SDL_ShowWindow(app->window);

    SDL_Log("App initialized");

    {
        int bbwidth = 0;
        int bbheight = 0;
        SDL_GetWindowSize(app->window, &app->WIDTH, &app->HEIGHT);
        SDL_GetWindowSizeInPixels(app->window, &bbwidth, &bbheight);
        SDL_Log("Window size: %ix%i", app->WIDTH, app->HEIGHT);
        SDL_Log("Backbuffer size: %ix%i", bbwidth, bbheight);
        if (app->WIDTH != bbwidth) {
            SDL_Log("This is a highdpi environment.");
        }
    }

    Menu_Init(app->menu, app->renderer, app);
    TicTacToe_Init(app->tic, app->renderer);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* state, SDL_Event* e) {
    auto* app = static_cast<App*>(state);

    switch (e->type) {
        case SDL_EVENT_QUIT:
            app->quitState = SDL_APP_SUCCESS;
            break;
        case SDL_EVENT_WINDOW_RESIZED:
            UpdateFPSText(app);
            UpdateScale(app);
            SDL_Log("Window size: %ix%i", app->WIDTH, app->HEIGHT);
            InitStars(static_cast<int>(app->WIDTH * app->HEIGHT / kStarCountBaseArea),
                      app->WIDTH, app->HEIGHT);
            break;
        case SDL_EVENT_MOUSE_MOTION:
            app->mouseX = e->motion.x;
            app->mouseY = e->motion.y;
            HandleSliderDrag(app, e->motion.x);
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            const float mx = e->button.x;
            const float my = e->button.y;
            auto hit = [&](SDL_FRect& r) {
                return mx >= r.x && mx <= r.x + r.w && my >= r.y && my <= r.y + r.h;
            };

            if (hit(app->volumeSlider.knob)) {
                app->volumeSlider.dragging = true;
            }
            if (hit(app->fpsSlider.knob)) {
                app->fpsSlider.dragging = true;
            }
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_UP:
            app->volumeSlider.dragging = false;
            app->fpsSlider.dragging = false;
            break;
        case SDL_EVENT_KEY_DOWN:
            HandleKeyDown(app, e->key.key);
            break;
        default:
            break;
    }

    if (app->state == GameState::TIC_TAC_TOE) {
        TicTacToe_HandleEvent(app->tic, e);
    }
    if (app->state == GameState::Menu) {
        Menu_HandleEvent(app->menu, e);
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* state) {
    auto* app = static_cast<App*>(state);

    const Uint64 now = SDL_GetPerformanceCounter();
    app->frameCount++;

    if (app->lastCounter == 0) {
        app->lastCounter = now;
    }

    const float elapsed = (now - app->lastCounter) /
                          static_cast<float>(SDL_GetPerformanceFrequency());

    if (elapsed >= 1.0f) {
        app->fps = app->frameCount / elapsed;
        app->frameCount = 0;
        app->lastCounter = now;
        UpdateFPSText(app);
    }

    SDL_SetRenderDrawColor(app->renderer, 14, 46, 92, 255);
    SDL_RenderClear(app->renderer);

    UpdateStars(app->WIDTH, app->HEIGHT, app->renderer);

    if (app->state == GameState::Menu) {
        Menu_Update(app->menu);
        Menu_Render(app->menu, app->renderer);
    } else if (app->state == GameState::TIC_TAC_TOE) {
        TicTacToe_Update(app->tic);
        TicTacToe_Render(app->tic, app->renderer);
    }

    SDL_RenderTexture(app->renderer, app->fpsTex, nullptr, &app->fpsRect);
    SDL_RenderPresent(app->renderer);

    if (app->limitFPS) {
        static Uint64 lastFrame = 0;
        const Uint64 ticksNow = SDL_GetTicks();
        const Uint32 frameTime = 1000 / app->targetFPS;
        const Uint32 delta = ticksNow - lastFrame;

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
