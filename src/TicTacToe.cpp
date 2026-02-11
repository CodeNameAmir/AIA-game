#include "TicTacToe.h"
#include "main.h"
#include "App.h"
#include <SDL3_image/SDL_image.h>
#include <filesystem>

static std::filesystem::path GetBasePath() {
    const char* base = SDL_GetBasePath();
    if (!base) return "";
    return std::filesystem::path(base);
}

int CheckWinnerWithLine(TicTacToe& game) {
    auto& b = game.board;

    // rows
    for (int y = 0; y < 3; y++) {
        if (b[y][0] && b[y][0] == b[y][1] && b[y][1] == b[y][2]) {
            game.hasWinLine = true;
            game.winLineStart = {
                game.cellRects[y][0].x + game.cellRects[y][0].w / 2,
                game.cellRects[y][0].y + game.cellRects[y][0].h / 2
            };
            game.winLineEnd = {
                game.cellRects[y][2].x + game.cellRects[y][2].w / 2,
                game.cellRects[y][2].y + game.cellRects[y][2].h / 2
            };
            return b[y][0];
        }
    }

    // columns
    for (int x = 0; x < 3; x++) {
        if (b[0][x] && b[0][x] == b[1][x] && b[1][x] == b[2][x]) {
            game.hasWinLine = true;
            game.winLineStart = {
                game.cellRects[0][x].x + game.cellRects[0][x].w / 2,
                game.cellRects[0][x].y + game.cellRects[0][x].h / 2
            };
            game.winLineEnd = {
                game.cellRects[2][x].x + game.cellRects[2][x].w / 2,
                game.cellRects[2][x].y + game.cellRects[2][x].h / 2
            };
            return b[0][x];
        }
    }

    // diagonal 
    if (b[0][0] && b[0][0] == b[1][1] && b[1][1] == b[2][2]) {
        game.hasWinLine = true;
        game.winLineStart = {
            game.cellRects[0][0].x + game.cellRects[0][0].w / 2,
            game.cellRects[0][0].y + game.cellRects[0][0].h / 2
        };
        game.winLineEnd = {
            game.cellRects[2][2].x + game.cellRects[2][2].w / 2,
            game.cellRects[2][2].y + game.cellRects[2][2].h / 2
        };
        return b[0][0];
    }

    // diagonal 
    if (b[0][2] && b[0][2] == b[1][1] && b[1][1] == b[2][0]) {
        game.hasWinLine = true;
        game.winLineStart = {
            game.cellRects[0][2].x + game.cellRects[0][2].w / 2,
            game.cellRects[0][2].y + game.cellRects[0][2].h / 2
        };
        game.winLineEnd = {
            game.cellRects[2][0].x + game.cellRects[2][0].w / 2,
            game.cellRects[2][0].y + game.cellRects[2][0].h / 2
        };
        return b[0][2];
    }

    return 0;
}

bool IsBoardFull(int board[3][3]) {
    for (int y = 0; y < 3; y++)
        for (int x = 0; x < 3; x++)
            if (board[y][x] == 0)
                return false;
    return true;
}

void TicTacToe_Init(TicTacToe& game, SDL_Renderer* renderer, App* app) {
    auto basePath = GetBasePath();
    game.app=app;
    UIItem replay; 
    //Win Card
    game.winCardBaseRect = {game.app->WIDTH / 2.f - 220,game.app->HEIGHT / 2.f - 150,440,300};
    game.winCardRect = game.winCardBaseRect;
    game.winCardtextbaseRect = {game.winCardRect.x + (game.winCardRect.w - 187) / 2,game.winCardRect.y + 18 * game.app->scale,187,44};
    game.winCardtextRect = game.winCardtextbaseRect;
    game.winCardReplaybaseRect = {game.winCardRect.x + game.winCardRect.w / 2 - 100 * game.app->scale,game.winCardRect.y + game.winCardRect.h - 80 * game.app->scale,200 * game.app->scale,50 * game.app->scale};
    // game.winCardReplayRect = game.winCardReplaybaseRect;
    replay.rect = replay.baserect= game.winCardReplaybaseRect;

    //sidemenuitem background
    game.sidemenuitembgTex = LoadIcon(renderer,(basePath / "icons/sidemenuitem.png").string().c_str());
    SDL_SetTextureAlphaMod(game.sidemenuitembgTex, 100);
    game.sidemenuitembgbaseRect={app->WIDTH-84*app->scale,472,313,664};
    game.sidemenuitembgRect=game.sidemenuitembgbaseRect;
    //logo image
    game.logoTex = LoadIcon(renderer,(basePath / "logo.png").string().c_str());
    game.logobaseRect = {8,(float)game.app->HEIGHT-57,110,53};
    game.logoRect = game.logobaseRect;
    game.logobgbaseRect = {0,(float)game.app->HEIGHT-59,125,59};
    game.logobgRect = game.logobgbaseRect;
    game.logobgTex = LoadIcon(renderer,(basePath / "logobg.png").string().c_str());
    SDL_SetTextureAlphaMod(game.logobgTex, 100);
    //turn : massage
    const std::string_view text = "Turn :";
    SDL_Surface* msgSurf = TTF_RenderText_Solid(app->font, text.data(), text.length(), {255,255,255,255});
    game.turnTex = SDL_CreateTextureFromSurface(app->renderer, msgSurf);
    SDL_DestroySurface(msgSurf);
    // auto props = SDL_GetTextureProperties(game.turnTex);
    float bw, bh;
    SDL_GetTextureSize(game.turnTex, &bw, &bh);
    game.turnbaseRect = {(280-bw)/2,60,bw,bh};    
    game.turnRect = game.turnbaseRect;
    //turn bg image
    game.turnbgTex = LoadIcon(renderer,(basePath / "tictactoepic/turnbg.png").string().c_str());
    // SDL_GetTextureSize(game.turnbgTex, &bw, &bh);
    game.turnbgbaseRect = {(280-170)/2,145,170,170};
    game.turnbgRect = game.turnbgbaseRect;
    //turn image
    game.turnimagebaseRect = {(280-125)/2,145+(170-125)/2,125,125};
    game.turnimageRect = game.turnimagebaseRect;
    //cell frame 
    float startX = 300;
    float startY = 0;
    float cellSize = 240;

    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            game.cellBaseRects[y][x] = {
                startX + x * cellSize,
                startY + y * cellSize,
                cellSize,
                cellSize
            };
            game.cellRects[y][x] = game.cellBaseRects[y][x];
        }
    }
    game.ximageTex=LoadIcon(renderer,(basePath / "tictactoepic/x.png").string().c_str());
    game.oimageTex=LoadIcon(renderer,(basePath / "tictactoepic/o.png").string().c_str());

    
}
void tic_switched(TicTacToe& game){
    
    ApplyUIScale(game.app->scale,game.logoRect,game.logobaseRect);
    ApplyUIScale(game.app->scale,game.logobgRect,game.logobgbaseRect);
    ApplyUIScale(game.app->scale,game.turnRect,game.turnbaseRect);
    ApplyUIScale(game.app->scale,game.turnbgRect,game.turnbgbaseRect);
    ApplyUIScale(game.app->scale,game.turnimageRect,game.turnimagebaseRect);
    //x-o frame
    ApplyUIScale(game.app->scale,game.xoFrame,game.xobaseFrame);
    ApplyUIScale(game.app->scale,game.vertical_line_1,game.base_vertical_line_1);
    ApplyUIScale(game.app->scale,game.vertical_line_2,game.base_vertical_line_2);
    ApplyUIScale(game.app->scale,game.horizontal_line_1,game.base_horizontal_line_1);
    ApplyUIScale(game.app->scale,game.horizontal_line_2,game.base_horizontal_line_2);
    ApplyUIScale(game.app->scale, game.winCardRect, game.winCardBaseRect);
    ApplyUIScale(game.app->scale, game.winCardtextRect, game.winCardtextbaseRect);
    ApplyUIScale(game.app->scale, game.winCardReplayRect, game.winCardReplaybaseRect);

    for (int y = 0; y < 3; y++)
    for (int x = 0; x < 3; x++)
        ApplyUIScale(game.app->scale,game.cellRects[y][x],game.cellBaseRects[y][x]);
    CheckWinnerWithLine(game);

    game.logoRect.y=game.app->HEIGHT-57*game.app->scale;
    game.logobgRect.y=game.app->HEIGHT-59*game.app->scale;
    game.sidemenuitembgRect.x=game.app->WIDTH-84*game.app->scale;

}
bool PointInRect(int x, int y, const SDL_FRect& r) {
    return x >= r.x && x <= r.x + r.w &&y >= r.y && y <= r.y + r.h;
}
void ResetGame(TicTacToe& game) {
    memset(game.board, 0, sizeof(game.board));
    game.xTurn = true;
    game.gameOver = false;
    game.winner = 0;
    game.hasWinLine = false;

}

void TicTacToe_HandleEvent(TicTacToe& game, SDL_Event* e) {
    if (e->type == SDL_EVENT_WINDOW_RESIZED){
        tic_switched(game);
        // ApplyUIScale(game.app->scale,game.sidemenuitembgRect,game.sidemenuitembgbaseRect);
    }
    if (e->key.key == SDLK_R ) ResetGame(game);
    if (e->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        int mx = e->button.x;
        int my = e->button.y;
        if (game.gameOver) return;
        for (int y = 0; y < 3; y++) {
            for (int x = 0; x < 3; x++) {

                if (PointInRect(mx, my, game.cellRects[y][x]) &&
                    game.board[y][x] == 0) {

                    game.board[y][x] = game.xTurn ? 1 : 2;
                    game.xTurn = !game.xTurn;
                    
                    // check winner
                    int w = CheckWinnerWithLine(game);
                    if (w == 0) const char* txt = "PERFECT BALANCE";

                    if (w != 0) {
                        game.gameOver = true;
                        game.winner = w;
                        const char* txt = (w == 1) ? "X TAKES IT" : "O TAKES IT";
                        SDL_Surface* s = TTF_RenderText_Blended(game.app->font, txt, SDL_strlen(txt),{255,255,255});
                        game.winTextTex = SDL_CreateTextureFromSurface(game.app->renderer, s);
                        SDL_DestroySurface(s);
                        SDL_Log("WINNER: %s", w == 1 ? "X" : "O");
                        return;
                    }
                    // check draw
                    if (IsBoardFull(game.board)) {
                        game.gameOver = true;
                        game.winner = 0;
                        SDL_Log("DRAW!");
                    }

                    SDL_Log("Clicked cell: %d , %d\t turn :%s", x, y,(!game.xTurn)?"x":"y");
                    return; 
                }
            }
        }
    }
    

}
void TicTacToe_Update(TicTacToe&) {
    // منطق بازی بعداً اینجا میاد
}

void TicTacToe_Render(TicTacToe& game, SDL_Renderer* renderer) {
    SDL_RenderTexture(renderer, game.logobgTex, nullptr, &game.logobgRect);
    SDL_RenderTexture(renderer, game.logoTex, nullptr, &game.logoRect);
    SDL_RenderTexture(renderer, game.sidemenuitembgTex, nullptr, &game.sidemenuitembgRect);
    game.sidemenuitembgRect.y=game.app->sideButtons[0].ui.rect.y-20;
    if(game.xTurn)SDL_RenderTexture(renderer, game.ximageTex, nullptr, &game.turnimageRect);
    if(!game.xTurn)SDL_RenderTexture(renderer, game.oimageTex, nullptr, &game.turnimageRect);
    SidePanel_Render(game.app, renderer);

    SDL_RenderTexture(renderer, game.turnTex, nullptr, &game.turnRect);
    SDL_RenderTexture(renderer, game.turnbgTex, nullptr, &game.turnbgRect);
    
    //x-o frame
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);
    SDL_RenderRect(renderer,&game.xoFrame);
    SDL_RenderFillRect(renderer,&game.vertical_line_1);
    SDL_RenderFillRect(renderer,&game.vertical_line_2);
    SDL_RenderFillRect(renderer,&game.horizontal_line_1);
    SDL_RenderFillRect(renderer,&game.horizontal_line_2);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 100);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 100);
    // SDL_Log("nobate %i",game.xTurn);
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {

            if (game.board[y][x] != 0) {

                SDL_FRect drawRect;
                drawRect.w = 180 * game.app->scale;
                drawRect.h = 180 * game.app->scale;
                drawRect.x =game.cellRects[y][x].x +(game.cellRects[y][x].w - drawRect.w) / 2;
                drawRect.y =game.cellRects[y][x].y +(game.cellRects[y][x].h - drawRect.h) / 2;

                if (game.board[y][x] == 1)
                    SDL_RenderTexture(renderer, game.ximageTex, nullptr, &drawRect);

                else if (game.board[y][x] == 2)
                    SDL_RenderTexture(renderer, game.oimageTex, nullptr, &drawRect);
            }
        }
    }

    if (game.hasWinLine) {
    float pulse = sin(SDL_GetTicks() / 750.f)/ 2.0 * 255;
    SDL_SetRenderDrawColor(renderer, 50, 171, 141, pulse);
        
    float x1 = game.winLineStart.x;
    float y1 = game.winLineStart.y;
    float x2 = game.winLineEnd.x;
    float y2 = game.winLineEnd.y;

    float dx = x2 - x1;
    float dy = y2 - y1;
    float len = sqrtf(dx*dx + dy*dy);

    float nx = -dy / len;
    float ny =  dx / len;

    float thickness = 6.0f * game.app->scale;

    for (int i = -thickness; i <= thickness; i++) {
        SDL_RenderLine(renderer,
            x1 + nx * i,
            y1 + ny * i,
            x2 + nx * i,
            y2 + ny * i
        );
    }
    
}
if (game.gameOver) {
    // float tw, th;
    // SDL_GetTextureSize(game.winTextTex, &tw, &th);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 170); // overlay
    SDL_FRect overlay = {0, 0, (float)game.app->WIDTH, (float)game.app->HEIGHT};
    SDL_RenderFillRect(renderer, &overlay);

    SDL_SetRenderDrawColor(renderer, 20, 40, 80, 240);
    SDL_RenderFillRect(renderer, &game.winCardRect);
    SDL_SetRenderDrawColor(renderer, 50, 171, 141, 255);
    SDL_RenderRect(renderer, &game.winCardRect);
    // SDL_FRect textRect = {game.winCardRect.x + (game.winCardRect.w - tw) / 2,game.winCardRect.y + 18 * game.app->scale,tw*game.app->scale,th*game.app->scale};
    SDL_RenderTexture(renderer, game.winTextTex, nullptr, &game.winCardtextRect);
    SDL_SetRenderDrawColor(renderer, 50, 171, 141, 255);
    SDL_RenderFillRect(renderer, &game.winCardReplayRect);
    

}



}

void TicTacToe_Quit(TicTacToe& game) {
    SDL_DestroyTexture(game.logoTex);
    SDL_DestroyTexture(game.logobgTex);
    SDL_DestroyTexture(game.turnTex);
    SDL_DestroyTexture(game.turnbgTex);
}
