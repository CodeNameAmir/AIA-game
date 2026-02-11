#include "TicTacToe.h"

#include "App.h"
#include "main.h"

#include <cmath>
#include <cstring>
#include <filesystem>
#include <string_view>

namespace {

constexpr int kBoardSize = 3;
constexpr float kBoardStartX = 300.0f;
constexpr float kBoardStartY = 0.0f;
constexpr float kCellSize = 240.0f;
constexpr float kMarkSize = 180.0f;
constexpr float kWinLineThickness = 6.0f;

std::filesystem::path GetBasePath() {
    const char* base = SDL_GetBasePath();
    if (!base) {
        return "";
    }

    return std::filesystem::path(base);
}

bool IsCellInsidePoint(int x, int y, const SDL_FRect& rect) {
    return x >= rect.x && x <= rect.x + rect.w &&
           y >= rect.y && y <= rect.y + rect.h;
}

void UpdateWinTextTexture(TicTacToe& game, const char* text) {
    SDL_DestroyTexture(game.winTextTex);
    game.winTextTex = nullptr;

    SDL_Surface* surface = TTF_RenderText_Blended(game.app->font, text, SDL_strlen(text), {255, 255, 255});
    game.winTextTex = SDL_CreateTextureFromSurface(game.app->renderer, surface);
    SDL_DestroySurface(surface);
}

bool IsBoardFull(const int board[kBoardSize][kBoardSize]) {
    for (int y = 0; y < kBoardSize; ++y) {
        for (int x = 0; x < kBoardSize; ++x) {
            if (board[y][x] == 0) {
                return false;
            }
        }
    }

    return true;
}

void SetWinLine(TicTacToe& game, const SDL_FRect& startRect, const SDL_FRect& endRect) {
    game.hasWinLine = true;
    game.winLineStart = {startRect.x + startRect.w / 2, startRect.y + startRect.h / 2};
    game.winLineEnd = {endRect.x + endRect.w / 2, endRect.y + endRect.h / 2};
}

int CheckWinnerWithLine(TicTacToe& game) {
    auto& board = game.board;

    for (int y = 0; y < kBoardSize; ++y) {
        if (board[y][0] != 0 && board[y][0] == board[y][1] && board[y][1] == board[y][2]) {
            SetWinLine(game, game.cellRects[y][0], game.cellRects[y][2]);
            return board[y][0];
        }
    }

    for (int x = 0; x < kBoardSize; ++x) {
        if (board[0][x] != 0 && board[0][x] == board[1][x] && board[1][x] == board[2][x]) {
            SetWinLine(game, game.cellRects[0][x], game.cellRects[2][x]);
            return board[0][x];
        }
    }

    if (board[0][0] != 0 && board[0][0] == board[1][1] && board[1][1] == board[2][2]) {
        SetWinLine(game, game.cellRects[0][0], game.cellRects[2][2]);
        return board[0][0];
    }

    if (board[0][2] != 0 && board[0][2] == board[1][1] && board[1][1] == board[2][0]) {
        SetWinLine(game, game.cellRects[0][2], game.cellRects[2][0]);
        return board[0][2];
    }

    return 0;
}

void UpdateLayoutWithScale(TicTacToe& game) {
    ApplyUIScale(game.app->scale, game.logoRect, game.logobaseRect);
    ApplyUIScale(game.app->scale, game.logobgRect, game.logobgbaseRect);
    ApplyUIScale(game.app->scale, game.turnRect, game.turnbaseRect);
    ApplyUIScale(game.app->scale, game.turnbgRect, game.turnbgbaseRect);
    ApplyUIScale(game.app->scale, game.turnimageRect, game.turnimagebaseRect);

    ApplyUIScale(game.app->scale, game.xoFrame, game.xobaseFrame);
    ApplyUIScale(game.app->scale, game.vertical_line_1, game.base_vertical_line_1);
    ApplyUIScale(game.app->scale, game.vertical_line_2, game.base_vertical_line_2);
    ApplyUIScale(game.app->scale, game.horizontal_line_1, game.base_horizontal_line_1);
    ApplyUIScale(game.app->scale, game.horizontal_line_2, game.base_horizontal_line_2);

    ApplyUIScale(game.app->scale, game.winCardRect, game.winCardBaseRect);
    ApplyUIScale(game.app->scale, game.winCardtextRect, game.winCardtextbaseRect);
    ApplyUIScale(game.app->scale, game.winCardReplayRect, game.winCardReplaybaseRect);

    for (int y = 0; y < kBoardSize; ++y) {
        for (int x = 0; x < kBoardSize; ++x) {
            ApplyUIScale(game.app->scale, game.cellRects[y][x], game.cellBaseRects[y][x]);
        }
    }

    CheckWinnerWithLine(game);

    game.logoRect.y = game.app->HEIGHT - 57 * game.app->scale;
    game.logobgRect.y = game.app->HEIGHT - 59 * game.app->scale;
    game.sidemenuitembgRect.x = game.app->WIDTH - 84 * game.app->scale;
}

void RenderBoardMarks(TicTacToe& game, SDL_Renderer* renderer) {
    for (int y = 0; y < kBoardSize; ++y) {
        for (int x = 0; x < kBoardSize; ++x) {
            if (game.board[y][x] == 0) {
                continue;
            }

            SDL_FRect drawRect{};
            drawRect.w = kMarkSize * game.app->scale;
            drawRect.h = kMarkSize * game.app->scale;
            drawRect.x = game.cellRects[y][x].x + (game.cellRects[y][x].w - drawRect.w) / 2;
            drawRect.y = game.cellRects[y][x].y + (game.cellRects[y][x].h - drawRect.h) / 2;

            SDL_Texture* texture = (game.board[y][x] == 1) ? game.ximageTex : game.oimageTex;
            SDL_RenderTexture(renderer, texture, nullptr, &drawRect);
        }
    }
}

void RenderWinLine(TicTacToe& game, SDL_Renderer* renderer) {
    if (!game.hasWinLine) {
        return;
    }

    const float pulseAlpha = std::sin(SDL_GetTicks() / 750.0f) / 2.0f * 255.0f;
    SDL_SetRenderDrawColor(renderer, 50, 171, 141, static_cast<Uint8>(pulseAlpha));

    const float dx = game.winLineEnd.x - game.winLineStart.x;
    const float dy = game.winLineEnd.y - game.winLineStart.y;
    const float len = std::sqrt(dx * dx + dy * dy);

    const float nx = -dy / len;
    const float ny = dx / len;
    const float thickness = kWinLineThickness * game.app->scale;

    for (int i = -static_cast<int>(thickness); i <= static_cast<int>(thickness); ++i) {
        SDL_RenderLine(
            renderer,
            game.winLineStart.x + nx * i,
            game.winLineStart.y + ny * i,
            game.winLineEnd.x + nx * i,
            game.winLineEnd.y + ny * i);
    }
}

void RenderGameOverOverlay(TicTacToe& game, SDL_Renderer* renderer) {
    if (!game.gameOver) {
        return;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 170);
    SDL_FRect overlay = {0, 0, static_cast<float>(game.app->WIDTH), static_cast<float>(game.app->HEIGHT)};
    SDL_RenderFillRect(renderer, &overlay);

    SDL_SetRenderDrawColor(renderer, 20, 40, 80, 240);
    SDL_RenderFillRect(renderer, &game.winCardRect);

    SDL_SetRenderDrawColor(renderer, 50, 171, 141, 255);
    SDL_RenderRect(renderer, &game.winCardRect);

    SDL_RenderTexture(renderer, game.winTextTex, nullptr, &game.winCardtextRect);

    SDL_SetRenderDrawColor(renderer, 50, 171, 141, 255);
    SDL_RenderFillRect(renderer, &game.winCardReplayRect);
}

void ResetGame(TicTacToe& game) {
    std::memset(game.board, 0, sizeof(game.board));
    game.xTurn = true;
    game.gameOver = false;
    game.winner = 0;
    game.hasWinLine = false;

    UpdateWinTextTexture(game, "PERFECT BALANCE");
}

}  // namespace

void TicTacToe_Init(TicTacToe& game, SDL_Renderer* renderer, App* app) {
    auto basePath = GetBasePath();
    game.app = app;

    game.sidemenuitembgTex = LoadIcon(renderer, (basePath / "sidemenuitem.png").string().c_str());
    SDL_SetTextureAlphaMod(game.sidemenuitembgTex, 100);

    game.logoTex = LoadIcon(renderer, (basePath / "logo.png").string().c_str());
    game.logobgTex = LoadIcon(renderer, (basePath / "logobg.png").string().c_str());
    SDL_SetTextureAlphaMod(game.logobgTex, 100);

    game.turnbgTex = LoadIcon(renderer, (basePath / "tictactoepic/turnbg.png").string().c_str());
    game.ximageTex = LoadIcon(renderer, (basePath / "tictactoepic/x.png").string().c_str());
    game.oimageTex = LoadIcon(renderer, (basePath / "tictactoepic/o.png").string().c_str());

    game.winCardBaseRect = {game.app->WIDTH / 2.0f - 220, game.app->HEIGHT / 2.0f - 150, 440, 300};
    game.winCardRect = game.winCardBaseRect;
    game.winCardtextbaseRect = {game.winCardRect.x + (game.winCardRect.w - 187) / 2, game.winCardRect.y + 18 * game.app->scale, 187, 44};
    game.winCardtextRect = game.winCardtextbaseRect;
    game.winCardReplaybaseRect = {
        game.winCardRect.x + game.winCardRect.w / 2 - 100 * game.app->scale,
        game.winCardRect.y + game.winCardRect.h - 80 * game.app->scale,
        200 * game.app->scale,
        50 * game.app->scale,
    };
    game.winCardReplayRect = game.winCardReplaybaseRect;

    game.sidemenuitembgbaseRect = {app->WIDTH - 84 * app->scale, 472, 313, 664};
    game.sidemenuitembgRect = game.sidemenuitembgbaseRect;

    game.logobaseRect = {8, static_cast<float>(game.app->HEIGHT - 57), 110, 53};
    game.logoRect = game.logobaseRect;
    game.logobgbaseRect = {0, static_cast<float>(game.app->HEIGHT - 59), 125, 59};
    game.logobgRect = game.logobgbaseRect;

    const std::string_view turnText = "Turn :";
    SDL_Surface* turnSurface = TTF_RenderText_Solid(app->font, turnText.data(), turnText.length(), {255, 255, 255, 255});
    game.turnTex = SDL_CreateTextureFromSurface(app->renderer, turnSurface);
    SDL_DestroySurface(turnSurface);

    float turnWidth = 0.0f;
    float turnHeight = 0.0f;
    SDL_GetTextureSize(game.turnTex, &turnWidth, &turnHeight);
    game.turnbaseRect = {(280 - turnWidth) / 2, 60, turnWidth, turnHeight};
    game.turnRect = game.turnbaseRect;

    game.turnbgbaseRect = {(280 - 170) / 2, 145, 170, 170};
    game.turnbgRect = game.turnbgbaseRect;

    game.turnimagebaseRect = {(280 - 125) / 2, 145 + (170 - 125) / 2, 125, 125};
    game.turnimageRect = game.turnimagebaseRect;

    for (int y = 0; y < kBoardSize; ++y) {
        for (int x = 0; x < kBoardSize; ++x) {
            game.cellBaseRects[y][x] = {
                kBoardStartX + x * kCellSize,
                kBoardStartY + y * kCellSize,
                kCellSize,
                kCellSize,
            };
            game.cellRects[y][x] = game.cellBaseRects[y][x];
        }
    }

    ResetGame(game);
}

void tic_switched(TicTacToe& game) {
    UpdateLayoutWithScale(game);
}

void TicTacToe_HandleEvent(TicTacToe& game, SDL_Event* e) {
    if (e->type == SDL_EVENT_WINDOW_RESIZED) {
        tic_switched(game);
    }

    if (e->type == SDL_EVENT_KEY_DOWN && e->key.key == SDLK_R) {
        ResetGame(game);
    }

    if (e->type != SDL_EVENT_MOUSE_BUTTON_DOWN || e->button.button != SDL_BUTTON_LEFT || game.gameOver) {
        return;
    }

    const int mouseX = e->button.x;
    const int mouseY = e->button.y;

    for (int y = 0; y < kBoardSize; ++y) {
        for (int x = 0; x < kBoardSize; ++x) {
            if (!IsCellInsidePoint(mouseX, mouseY, game.cellRects[y][x]) || game.board[y][x] != 0) {
                continue;
            }

            game.board[y][x] = game.xTurn ? 1 : 2;
            game.xTurn = !game.xTurn;

            const int winner = CheckWinnerWithLine(game);
            if (winner != 0) {
                game.gameOver = true;
                game.winner = winner;
                UpdateWinTextTexture(game, winner == 1 ? "X TAKES IT" : "O TAKES IT");
                SDL_Log("WINNER: %s", winner == 1 ? "X" : "O");
                return;
            }

            if (IsBoardFull(game.board)) {
                game.gameOver = true;
                game.winner = 0;
                UpdateWinTextTexture(game, "PERFECT BALANCE");
                SDL_Log("DRAW!");
            }

            SDL_Log("Clicked cell: %d , %d\t turn :%s", x, y, (!game.xTurn) ? "x" : "y");
            return;
        }
    }
}

void TicTacToe_Update(TicTacToe&) {}

void TicTacToe_Render(TicTacToe& game, SDL_Renderer* renderer) {
    game.sidemenuitembgRect.y = game.app->sideButtons[0].ui.rect.y - 20;

    SDL_RenderTexture(renderer, game.logobgTex, nullptr, &game.logobgRect);
    SDL_RenderTexture(renderer, game.logoTex, nullptr, &game.logoRect);
    SDL_RenderTexture(renderer, game.sidemenuitembgTex, nullptr, &game.sidemenuitembgRect);

    SDL_RenderTexture(renderer, game.turnTex, nullptr, &game.turnRect);
    SDL_RenderTexture(renderer, game.turnbgTex, nullptr, &game.turnbgRect);
    SDL_RenderTexture(renderer, game.xTurn ? game.ximageTex : game.oimageTex, nullptr, &game.turnimageRect);

    SidePanel_Render(game.app, renderer);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);
    SDL_RenderRect(renderer, &game.xoFrame);
    SDL_RenderFillRect(renderer, &game.vertical_line_1);
    SDL_RenderFillRect(renderer, &game.vertical_line_2);
    SDL_RenderFillRect(renderer, &game.horizontal_line_1);
    SDL_RenderFillRect(renderer, &game.horizontal_line_2);

    RenderBoardMarks(game, renderer);
    RenderWinLine(game, renderer);
    RenderGameOverOverlay(game, renderer);
}

void TicTacToe_Quit(TicTacToe& game) {
    SDL_DestroyTexture(game.logoTex);
    SDL_DestroyTexture(game.logobgTex);
    SDL_DestroyTexture(game.turnTex);
    SDL_DestroyTexture(game.turnbgTex);
    SDL_DestroyTexture(game.ximageTex);
    SDL_DestroyTexture(game.oimageTex);
    SDL_DestroyTexture(game.sidemenuitembgTex);
    SDL_DestroyTexture(game.winTextTex);
}
