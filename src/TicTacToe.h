#pragma once
#include <SDL3/SDL.h>

struct TicTacToe {
    int board[3][3];
    bool xTurn;

    SDL_Texture* background = nullptr;
    SDL_FRect bgRect{0, 0, 0, 0};
};

void TicTacToe_Init(TicTacToe& game, SDL_Renderer* renderer);
void TicTacToe_Update(TicTacToe& game);
void TicTacToe_Render(TicTacToe& game, SDL_Renderer* renderer);
void TicTacToe_HandleEvent(TicTacToe& game, SDL_Event* e);
void TicTacToe_Quit(TicTacToe& game);