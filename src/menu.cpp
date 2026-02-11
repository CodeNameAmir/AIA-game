#include "menu.h"

#include "App.h"
#include "main.h"

#include <array>
#include <filesystem>
#include <string_view>

namespace {

constexpr int kMenuColumns = 4;
constexpr float kIconSize = 96.0f;
constexpr float kStartX = 180.0f;
constexpr float kStartY = 270.0f;
constexpr float kGapX = 220.0f;
constexpr float kGapY = 180.0f;

const std::array<const char*, 7> kMenuItemNames = {
    "Tic Tac Toe",
    "Sea Battle",
    "Yahtzee",
    "Memory Game",
    "Chess",
    "Snakes & Ladders",
    "Dots and Boxes",
};

const std::array<const char*, 7> kMenuIconPaths = {
    "menupic/tic_tac_toe.png",
    "menupic/chess.png",
    "menupic/memory.png",
    "menupic/seabattle.png",
    "menupic/snake_ladder.png",
    "menupic/yatzi.png",
    "menupic/dots_boxes.png",
};

std::filesystem::path GetBasePath() {
    const char* base = SDL_GetBasePath();
    if (!base) {
        return "";
    }

    return std::filesystem::path(base);
}

void InitMenuItems(App* app) {
    auto basePath = GetBasePath();
    TTF_Font* menuFont = TTF_OpenFont((basePath / "Inter-VariableFont.ttf").string().c_str(), 30);

    for (int i = 0; i < static_cast<int>(kMenuItemNames.size()); ++i) {
        const int col = i % kMenuColumns;
        const int row = i / kMenuColumns;

        MenuItem item{};
        item.type = static_cast<MenuItemType>(i);
        item.ui.baserect = {
            kStartX + col * kGapX,
            kStartY + row * kGapY,
            kIconSize,
            kIconSize,
        };
        item.ui.rect = item.ui.baserect;

        const std::string_view text = kMenuItemNames[i];
        SDL_Surface* textSurface = TTF_RenderText_Solid(menuFont, text.data(), text.length(), {105, 105, 105, 255});
        item.label = SDL_CreateTextureFromSurface(app->renderer, textSurface);
        SDL_DestroySurface(textSurface);

        auto props = SDL_GetTextureProperties(item.label);
        item.labelRect.w = static_cast<float>(SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_WIDTH_NUMBER, 0));
        item.labelRect.h = static_cast<float>(SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_HEIGHT_NUMBER, 0));

        item.ui.icon = LoadIcon(app->renderer, (basePath / kMenuIconPaths[i]).string().c_str());

        app->menuItems.push_back(item);
    }

    TTF_CloseFont(menuFont);
}

void RenderMenuItemIcon(SDL_Renderer* renderer, UIItem& item, float uiScale) {
    const float scale = item.hovered ? item.hoverScale : 1.0f;
    const Uint8 alpha = item.hovered ? item.hoverAlpha : item.normalAlpha;

    SDL_FRect drawRect = item.baserect;
    drawRect.w *= scale;
    drawRect.h *= scale;
    drawRect.x -= (drawRect.w - item.baserect.w) / 2;
    drawRect.y -= (drawRect.h - item.baserect.h) / 2;

    ApplyUIScale(uiScale, item.rect, drawRect);

    SDL_SetTextureAlphaMod(item.icon, alpha);
    SDL_RenderTexture(renderer, item.icon, nullptr, &item.rect);
}

void RenderHoveredLabel(Menu& game, const MenuItem& item) {
    SDL_FRect labelRect{
        item.ui.rect.x,
        item.ui.rect.y + item.ui.rect.h + 8,
        item.labelRect.w,
        item.labelRect.h,
    };
    labelRect.x = item.ui.rect.x + (item.ui.rect.w - labelRect.w) / 2;

    SDL_RenderTexture(game.app->renderer, item.label, nullptr, &labelRect);
}

}  // namespace

void Menu_Init(Menu& game, SDL_Renderer* renderer, App* app) {
    auto basePath = GetBasePath();

    game.app = app;
    game.logoTex = LoadIcon(renderer, (basePath / "logo.png").string().c_str());
    game.logobgTex = LoadIcon(renderer, (basePath / "logobg.png").string().c_str());
    game.sidemenuitembgTex = LoadIcon(renderer, (basePath / "sidemenuitem.png").string().c_str());

    SDL_SetTextureAlphaMod(game.logobgTex, 100);
    SDL_SetTextureAlphaMod(game.sidemenuitembgTex, 100);

    auto bgProps = SDL_GetTextureProperties(game.logobgTex);
    game.logobaseRect.w = static_cast<float>(SDL_GetNumberProperty(bgProps, SDL_PROP_TEXTURE_WIDTH_NUMBER, 0)) / 2;
    game.logobaseRect.h = static_cast<float>(SDL_GetNumberProperty(bgProps, SDL_PROP_TEXTURE_HEIGHT_NUMBER, 0)) / 2;
    game.logobaseRect.x = (app->WIDTH - game.logobaseRect.w) / 2;
    game.logobaseRect.y = 10;

    game.logobgbaseRect = {
        game.logobaseRect.x - 20,
        game.logobaseRect.y - 15,
        game.logobaseRect.w + 40,
        game.logobaseRect.h + 30,
    };

    game.logoRect = game.logobaseRect;
    game.logobgRect = game.logobgbaseRect;
    game.sidemenuitembgbaseRect = {app->WIDTH - 84 * app->scale, 472, 313, 664};
    game.sidemenuitembgRect = game.sidemenuitembgbaseRect;

    InitMenuItems(app);
}

void menu_switched(Menu& game) {
    ApplyUIScale(game.app->scale, game.logoRect, game.logobaseRect);
    ApplyUIScale(game.app->scale, game.logobgRect, game.logobgbaseRect);
    game.sidemenuitembgRect.x = game.app->WIDTH - 84 * game.app->scale;
}

void Menu_HandleEvent(Menu& game, SDL_Event* e) {
    if (e->type == SDL_EVENT_WINDOW_RESIZED) {
        menu_switched(game);
    }

    if (e->type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
        e->button.button == SDL_BUTTON_LEFT &&
        game.app->hoveredIndex != -1) {
        if (game.app->menuItems[game.app->hoveredIndex].type == MenuItemType::TicTacToe) {
            game.app->state = GameState::TIC_TAC_TOE;
        }
    }

    if (e->type == SDL_EVENT_MOUSE_MOTION) {
        game.app->hoveredIndex = -1;

        for (int i = 0; i < static_cast<int>(game.app->menuItems.size()); ++i) {
            auto& rect = game.app->menuItems[i].ui.rect;
            if (game.app->mouseX >= rect.x && game.app->mouseX <= rect.x + rect.w &&
                game.app->mouseY >= rect.y && game.app->mouseY <= rect.y + rect.h) {
                game.app->hoveredIndex = i;
                break;
            }
        }
    }

    if (e->type == SDL_EVENT_KEY_DOWN && e->key.key == SDLK_H) {
        if (game.app->state == GameState::TIC_TAC_TOE) {
            game.app->state = GameState::Menu;
            SDL_Log("Switched to Menu");
        } else if (game.app->state == GameState::Menu) {
            game.app->state = GameState::TIC_TAC_TOE;
            SDL_Log("Switched to Tic Tac Toe");
        }
    }
}

void Menu_Update(Menu&) {}

void Menu_Render(Menu& game, SDL_Renderer* renderer) {
    SDL_RenderTexture(renderer, game.logobgTex, nullptr, &game.logoRect);
    SDL_RenderTexture(renderer, game.logoTex, nullptr, &game.logoRect);
    SDL_RenderTexture(renderer, game.sidemenuitembgTex, nullptr, &game.sidemenuitembgRect);

    for (auto& item : game.app->menuItems) {
        UpdateHover(item.ui, game.app->mouseX, game.app->mouseY);
        RenderMenuItemIcon(renderer, item.ui, game.app->scale);

        if (item.ui.hovered) {
            RenderHoveredLabel(game, item);
        }
    }

    SidePanel_Render(game.app, renderer);
}

void Menu_Quit(Menu& game) {
    SDL_DestroyTexture(game.logoTex);
    SDL_DestroyTexture(game.logobgTex);
    SDL_DestroyTexture(game.sidemenuitembgTex);

    for (auto& item : game.app->menuItems) {
        SDL_DestroyTexture(item.ui.icon);
        SDL_DestroyTexture(item.label);
    }
}
