#pragma once
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
struct App;

struct TicTacToe {
    App* app = nullptr;

    SDL_Texture* sidemenuitembgTex = nullptr;
    SDL_FRect sidemenuitembgRect{};
    SDL_FRect sidemenuitembgbaseRect;


    SDL_Texture* logoTex = nullptr;
    SDL_FRect logoRect{};
    SDL_FRect logobaseRect=logoRect;
    SDL_Texture* logobgTex = nullptr;
    SDL_FRect logobgRect{};
    SDL_FRect logobgbaseRect=logobgRect;

    SDL_Texture* turnTex = nullptr;
    SDL_FRect turnRect{};
    SDL_FRect turnbaseRect{};

    SDL_Texture* turnbgTex = nullptr;
    SDL_FRect turnbgRect{};
    SDL_FRect turnbgbaseRect=turnbgRect;
    SDL_Texture* turnimageTex = nullptr;
    SDL_FRect turnimageRect{};
    SDL_FRect turnimagebaseRect=turnbgRect;

    SDL_Texture* ximageTex = nullptr;
    SDL_Texture* oimageTex = nullptr;

    SDL_FRect xobaseFrame{300.0f, 0.0f, 720.0f, 720.0f};
    SDL_FRect xoFrame= xobaseFrame;
    SDL_FRect vertical_line_1{540.0f, 30.0f, 3.0f, 660.0f};
    SDL_FRect vertical_line_2{780.0f, 30.0f, 3.0f, 660.0f};
    SDL_FRect horizontal_line_1{330.0f, 240.0f, 660.0f, 3.0f};
    SDL_FRect horizontal_line_2{330.0f, 480.0f, 660.0f, 3.0f};
    SDL_FRect base_vertical_line_1 = vertical_line_1;
    SDL_FRect base_vertical_line_2 = vertical_line_2;
    SDL_FRect base_horizontal_line_1 = horizontal_line_1;
    SDL_FRect base_horizontal_line_2 = horizontal_line_2;
    SDL_FRect cellBaseRects[3][3];
    SDL_FRect cellRects[3][3];
    int board[3][3];
    bool xTurn=true;
    bool gameOver = false;
    int winner = 0; // 0 = none, 1 = X, 2 = O
    bool hasWinLine = false;
    SDL_FPoint winLineStart;
    SDL_FPoint winLineEnd;
    SDL_FPoint winbaseLineStart;
    SDL_FPoint winbaseLineEnd;
    //win card
    SDL_FRect winCardBaseRect;
    SDL_FRect winCardRect;
    SDL_FRect winCardtextRect;
    SDL_FRect winCardtextbaseRect;
    SDL_Texture* winTextTex = nullptr;
    SDL_FRect winCardReplaybaseRect;
    SDL_FRect winCardReplayRect;



};

void TicTacToe_Init(TicTacToe& game, SDL_Renderer* renderer, App* app);
void TicTacToe_Update(TicTacToe& game);
void TicTacToe_Render(TicTacToe& game, SDL_Renderer* renderer);
void TicTacToe_HandleEvent(TicTacToe& game, SDL_Event* e);
void TicTacToe_Quit(TicTacToe& game);

void tic_switched(TicTacToe& game);