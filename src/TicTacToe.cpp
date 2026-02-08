#include "TicTacToe.h"
#include <SDL3_image/SDL_image.h>
#include <filesystem>

namespace {
std::filesystem::path GetBasePath() {
    const char* base = SDL_GetBasePath();
    if (!base) {
        return "";
    }
    return std::filesystem::path(base);
}
}

void TicTacToe_Init(TicTacToe& game, SDL_Renderer* renderer) {
    auto basePath = GetBasePath();
    auto bgPath = basePath / "tictactoe_bg.png";

    SDL_Surface* surf = IMG_Load(bgPath.string().c_str());
    if (!surf) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Failed to load tictactoe background: %s", SDL_GetError());
        return;
    }

    game.background = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_DestroySurface(surf);

    if (!game.background) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Failed to create background texture: %s", SDL_GetError());
        return;
    }

    for (auto& row : game.board) {
        for (int& cell : row) {
            cell = 0;
        }
    }
    game.xTurn = true;

    game.bgRect = {0.0f, 0.0f, 1280.0f, 720.0f};
}

void TicTacToe_HandleEvent(TicTacToe& game, SDL_Event* e) {
    if (e->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        const int cellX = e->button.x / 150;
        const int cellY = e->button.y / 150;

        if (cellX < 3 && cellY < 3 && game.board[cellY][cellX] == 0) {
            game.board[cellY][cellX] = game.xTurn ? 1 : 2;
            game.xTurn = !game.xTurn;
            SDL_Log("aaaaaaaaaaaa   %i", game.board[cellY][cellX]);
        }
    }
}

void TicTacToe_Update(TicTacToe&) {
    // منطق بازی بعداً اینجا میاد
}

void TicTacToe_Render(TicTacToe& game, SDL_Renderer* renderer) {
    if (game.background) {
        SDL_RenderTexture(renderer, game.background, nullptr, &game.bgRect);
    }
}

void TicTacToe_Quit(TicTacToe& game) {
    if (game.background) {
        SDL_DestroyTexture(game.background);
        game.background = nullptr;
    }
}
