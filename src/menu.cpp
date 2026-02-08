#include "Menu.h"
#include "main.h"
#include "App.h"

#include <array>
#include <filesystem>

namespace {
std::filesystem::path GetBasePath() {
    const char* base = SDL_GetBasePath();
    if (!base) {
        return "";
    }
    return std::filesystem::path(base);
}

constexpr std::array<const char*, 7> kMenuItemNames = {
    "Tic Tac Toe",
    "Sea Battle",
    "Yahtzee",
    "Memory Game",
    "Chess",
    "Snakes & Ladders",
    "Dots and Boxes"
};

constexpr std::array<const char*, 7> kMenuIcons = {
    "icons/tic_tac_toe.png",
    "icons/chess.png",
    "icons/memory.png",
    "icons/seabattle.png",
    "icons/snake_ladder.png",
    "icons/yatzi.png",
    "icons/dots_boxes.png"
};

void ApplyUIScale(float scale, SDL_FRect& item, const SDL_FRect& base) {
    item.x = base.x * scale;
    item.y = base.y * scale;
    item.w = base.w * scale;
    item.h = base.h * scale;
}

SDL_Texture* LoadIcon(SDL_Renderer* renderer, const std::string& path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        SDL_Log("Failed to load %s", path.c_str());
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    return texture;
}

void RenderUIItem(SDL_Renderer* renderer, UIItem& item, float uiScale) {
    const float scale = item.hovered ? item.hoverScale : 1.0f;
    const Uint8 alpha = item.hovered ? item.hoverAlpha : item.normalAlpha;

    SDL_FRect dst = item.baserect;
    dst.w *= scale;
    dst.h *= scale;
    dst.x -= (dst.w - item.baserect.w) / 2;
    dst.y -= (dst.h - item.baserect.h) / 2;

    ApplyUIScale(uiScale, item.rect, dst);

    SDL_SetTextureAlphaMod(item.icon, alpha);
    SDL_RenderTexture(renderer, item.icon, nullptr, &item.rect);
}

void RenderSidePanelItem(SDL_Renderer* renderer, UIItem& item, float uiScale, App* app, Menu& game) {
    const float scale = item.hovered ? item.hoverScale : 1.0f;
    const Uint8 alpha = item.hovered ? item.hoverAlpha : item.normalAlpha;

    SDL_FRect dst = item.baserect;
    dst.w *= scale;
    dst.h *= scale;
    dst.x -= (dst.w - item.baserect.w) / 2;
    dst.y -= (dst.h - item.baserect.h) / 2;

    ApplyUIScale(uiScale, item.rect, dst);
    item.rect.x = app->WIDTH - 70 * app->scale;
    item.rect.y = app->HEIGHT - item.spacey * app->scale;
    game.sidemenuitembgRect.y = app->sideButtons[0].ui.rect.y - 20;

    SDL_SetTextureAlphaMod(item.icon, alpha);
    SDL_RenderTexture(renderer, item.icon, nullptr, &item.rect);
}

bool PointInRect(float mx, float my, const SDL_FRect& rect) {
    return mx >= rect.x && mx <= rect.x + rect.w &&
           my >= rect.y && my <= rect.y + rect.h;
}

void UpdateHover(UIItem& item, float mx, float my) {
    item.hovered = PointInRect(mx, my, item.rect);
}

void InitMenuItems(App* app, size_t count) {
    constexpr int kCols = 4;
    constexpr float kIconSize = 96.0f;
    constexpr float kStartX = 180.0f;
    constexpr float kStartY = 270.0f;
    constexpr float kGapX = 220.0f;
    constexpr float kGapY = 180.0f;

    auto basePath = GetBasePath();
    TTF_Font* font = TTF_OpenFont((basePath / "Inter-VariableFont.ttf").string().c_str(), 30);

    app->menuItems.clear();
    app->menuItems.reserve(count);

    for (size_t i = 0; i < count; i++) {
        const int col = static_cast<int>(i % kCols);
        const int row = static_cast<int>(i / kCols);

        MenuItem item;
        item.type = static_cast<MenuItemType>(i);
        item.ui.baserect = {
            kStartX + col * kGapX,
            kStartY + row * kGapY,
            kIconSize,
            kIconSize
        };
        item.ui.rect = item.ui.baserect;
        app->menuItems.push_back(item);

        const std::string_view text = kMenuItemNames[i];
        SDL_Surface* msgSurf = TTF_RenderText_Solid(font, text.data(), text.length(),
                                                    {105, 105, 105, 255});
        app->menuItems[i].label = SDL_CreateTextureFromSurface(app->renderer, msgSurf);
        SDL_DestroySurface(msgSurf);
        auto props = SDL_GetTextureProperties(app->menuItems[i].label);
        app->menuItems[i].labelRect.w = static_cast<float>(
            SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_WIDTH_NUMBER, 0));
        app->menuItems[i].labelRect.h = static_cast<float>(
            SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_HEIGHT_NUMBER, 0));
    }

    TTF_CloseFont(font);
}

void InitSidePanel(App* app) {
    constexpr float kGap = 20.0f;
    constexpr float kSize = 56.0f;
    constexpr float kRightMargin = 20.0f;
    constexpr float kBottomMargin = 20.0f;

    const float panelX = 1280 - kRightMargin - kSize;
    const float startY = 720 - kBottomMargin - (kSize * 3 + kGap * 2);

    app->sideButtons.clear();
    app->sideButtons.reserve(3);

    for (int i = 0; i < 3; i++) {
        SideButton button{};
        button.ui.baserect = {
            panelX,
            startY + i * (kSize + kGap),
            kSize,
            kSize
        };
        button.ui.spacey = app->HEIGHT - button.ui.baserect.y;
        app->sideButtons.push_back(button);
    }

    auto basePath = GetBasePath();
    app->sideButtons[0].ui.icon = LoadIcon(app->renderer, (basePath / "icons/about.png").string());
    app->sideButtons[1].ui.icon = LoadIcon(app->renderer, (basePath / "icons/setting.png").string());
    app->sideButtons[2].ui.icon = LoadIcon(app->renderer, (basePath / "icons/exit.png").string());
}

} // namespace

void Menu_Init(Menu& game, SDL_Renderer* renderer, App* app) {
    auto basePath = GetBasePath();
    game.app = app;

    SDL_Surface* logoSurf = IMG_Load((basePath / "logo.png").string().c_str());
    if (!logoSurf) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: %s", "Load logo.png", SDL_GetError());
    }
    game.logoTex = SDL_CreateTextureFromSurface(renderer, logoSurf);
    SDL_DestroySurface(logoSurf);

    SDL_Surface* logobgSurf = IMG_Load((basePath / "logobg.png").string().c_str());
    if (!logobgSurf) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: %s", "Load logobg.png", SDL_GetError());
    }
    game.logobgTex = SDL_CreateTextureFromSurface(renderer, logobgSurf);
    SDL_SetTextureAlphaMod(game.logobgTex, 100);
    SDL_DestroySurface(logobgSurf);

    auto props = SDL_GetTextureProperties(game.logobgTex);
    game.logobaseRect.w = static_cast<float>(
        SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_WIDTH_NUMBER, 0)) / 2;
    game.logobaseRect.h = static_cast<float>(
        SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_HEIGHT_NUMBER, 0)) / 2;

    game.logobaseRect.x = (app->WIDTH - game.logobaseRect.w) / 2;
    game.logobaseRect.y = 10;
    game.logobgbaseRect.x = game.logobaseRect.x - 20;
    game.logobgbaseRect.y = game.logobaseRect.y - 15;
    game.logobgbaseRect.w = game.logobaseRect.w + 40;
    game.logobgbaseRect.h = game.logobaseRect.h + 30;
    game.logoRect = game.logobaseRect;
    game.logobgRect = game.logobgbaseRect;

    InitMenuItems(app, kMenuIcons.size());
    InitSidePanel(app);

    SDL_Surface* sidemenuitembgSurf = IMG_Load((basePath / "icons/sidemenuitem.png").string().c_str());
    if (!sidemenuitembgSurf) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: %s", "Load sidemenuitem.png", SDL_GetError());
    }
    game.sidemenuitembgTex = SDL_CreateTextureFromSurface(renderer, sidemenuitembgSurf);
    SDL_SetTextureAlphaMod(game.sidemenuitembgTex, 100);
    SDL_DestroySurface(sidemenuitembgSurf);

    game.sidemenuitembgbaseRect = {app->WIDTH - 84 * app->scale,
                                   app->sideButtons[0].ui.baserect.y - 20,
                                   313,
                                   664};
    game.sidemenuitembgRect = game.sidemenuitembgbaseRect;

    for (size_t i = 0; i < app->menuItems.size(); i++) {
        app->menuItems[i].ui.icon = LoadIcon(app->renderer, (basePath / kMenuIcons[i]).string());
    }
}

void Menu_HandleEvent(Menu& game, SDL_Event* e) {
    if (e->type == SDL_EVENT_WINDOW_RESIZED) {
        ApplyUIScale(game.app->scale, game.logoRect, game.logobaseRect);
        ApplyUIScale(game.app->scale, game.logobgRect, game.logobgbaseRect);
        game.sidemenuitembgRect.x = game.app->WIDTH - 84 * game.app->scale;
    }

    if (e->type == SDL_EVENT_MOUSE_BUTTON_DOWN && e->button.button == SDL_BUTTON_LEFT &&
        game.app->hoveredIndex != -1) {
        switch (game.app->menuItems[game.app->hoveredIndex].type) {
            case MenuItemType::TicTacToe:
                game.app->state = GameState::TIC_TAC_TOE;
                break;
            default:
                break;
        }
    }

    if (e->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        for (size_t i = 0; i < game.app->sideButtons.size(); i++) {
            auto& rect = game.app->sideButtons[i].ui.rect;
            if (game.app->mouseX >= rect.x && game.app->mouseX <= rect.x + rect.w &&
                game.app->mouseY >= rect.y && game.app->mouseY <= rect.y + rect.h) {
                if (i == 0) {
                    SDL_Log("about");
                }
                if (i == 1) {
                    SDL_Log("setting");
                }
                if (i == 2) {
                    game.app->quitState = SDL_APP_SUCCESS;
                }
            }
        }
    }

    if (e->type == SDL_EVENT_MOUSE_MOTION) {
        game.app->hoveredIndex = -1;

        for (size_t i = 0; i < game.app->menuItems.size(); i++) {
            auto& rect = game.app->menuItems[i].ui.rect;
            if (game.app->mouseX >= rect.x && game.app->mouseX <= rect.x + rect.w &&
                game.app->mouseY >= rect.y && game.app->mouseY <= rect.y + rect.h) {
                game.app->hoveredIndex = static_cast<int>(i);
                break;
            }
        }

        for (size_t i = 0; i < game.app->sideButtons.size(); i++) {
            auto& rect = game.app->sideButtons[i].ui.baserect;
            if (game.app->mouseX >= rect.x && game.app->mouseX <= rect.x + rect.w &&
                game.app->mouseY >= rect.y && game.app->mouseY <= rect.y + rect.h) {
                if (i == 0) {
                    SDL_Log("about");
                }
                if (i == 1) {
                    SDL_Log("setting");
                }
                if (i == 2) {
                    SDL_Log("exit");
                }
            }
        }
    }

    if (e->type == SDL_EVENT_KEY_DOWN) {
        SDL_Log("pressed key : %s", SDL_GetKeyName(e->key.key));
        switch (e->key.key) {
            case SDLK_H:
                if (game.app->state == GameState::TIC_TAC_TOE) {
                    game.app->state = GameState::Menu;
                    SDL_Log("Switched to Menu");
                } else if (game.app->state == GameState::Menu) {
                    game.app->state = GameState::TIC_TAC_TOE;
                    SDL_Log("Switched to Tic Tac Toe");
                }
                break;
            default:
                break;
        }
    }
}

void Menu_Update(Menu&) {
    // منطق بازی بعداً اینجا میاد
}

void Menu_Render(Menu& game, SDL_Renderer* renderer) {
    SDL_RenderTexture(renderer, game.logobgTex, nullptr, &game.logoRect);
    SDL_RenderTexture(renderer, game.logoTex, nullptr, &game.logoRect);
    SDL_RenderTexture(renderer, game.sidemenuitembgTex, nullptr, &game.sidemenuitembgRect);

    for (auto& item : game.app->menuItems) {
        UpdateHover(item.ui, game.app->mouseX, game.app->mouseY);
        RenderUIItem(game.app->renderer, item.ui, game.app->scale);

        if (item.ui.hovered) {
            SDL_FRect lr{
                item.ui.rect.x,
                item.ui.rect.y + item.ui.rect.h + 8,
                item.labelRect.w,
                item.labelRect.h
            };
            lr.x = item.ui.rect.x + (item.ui.rect.w - lr.w) / 2;
            SDL_RenderTexture(game.app->renderer, item.label, nullptr, &lr);
        }
    }

    for (auto& button : game.app->sideButtons) {
        UpdateHover(button.ui, game.app->mouseX, game.app->mouseY);
        RenderSidePanelItem(renderer, button.ui, game.app->scale, game.app, game.app->menu);
    }
}

void Menu_Quit(Menu& game) {
    SDL_DestroyTexture(game.logoTex);
    SDL_DestroyTexture(game.logobgTex);
    SDL_DestroyTexture(game.sidemenuitembgTex);
}
