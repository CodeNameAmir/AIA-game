#include "Menu.h"
#include "main.h"
#include "App.h"


#include <filesystem>

static std::filesystem::path GetBasePath() {
    
    const char* base = SDL_GetBasePath();
    if (!base) return "";
    return std::filesystem::path(base);
}
static const char* MenuItemNames[] = {
    "Tic Tac Toe",
    "Sea Battle",
    "Yahtzee",
    "Memory Game",
    "Chess",
    "Snakes & Ladders",
    "Dots and Boxes"
};



void InitMenu(App* app,int T) {
    auto basePath = GetBasePath();

    const int cols = 4;
    const int rows = 2;

    float iconSize = 96.0f;
    float startX = 180.0f;
    float startY = 270.0f;
    float gapX = 220.0f;
    float gapY = 180.0f;
    TTF_Font* font = TTF_OpenFont((basePath / "Inter-VariableFont.ttf").string().c_str(), 30);
    for (int i = 0; i < T; i++) {
        int col = i % cols;
        int row = i / cols;

        MenuItem item;
        item.type = (MenuItemType)i;
        item.ui.baserect = {
            startX + col * gapX,
            startY + row * gapY,
            iconSize,
            iconSize
        };
        item.ui.rect = item.ui.baserect;
        app->menuItems.push_back(item);

        const std::string_view text = MenuItemNames[i];
        SDL_Surface* msgSurf = TTF_RenderText_Solid(font, text.data(), text.length(), {105,105,105,255});
        app->menuItems[i].label = SDL_CreateTextureFromSurface(app->renderer, msgSurf);
        SDL_DestroySurface(msgSurf);
        auto props = SDL_GetTextureProperties(app->menuItems[i].label);
        app->menuItems[i].labelRect.w = (float)SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_WIDTH_NUMBER, 0);
        app->menuItems[i].labelRect.h = (float)SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_HEIGHT_NUMBER, 0);
    }
    
}




void RenderUIItem(SDL_Renderer* r, UIItem& item, float uiScale) {
    float scale = item.hovered ? item.hoverScale : 1.0f;
    Uint8 alpha = item.hovered ? item.hoverAlpha : item.normalAlpha;

    SDL_FRect dst = item.baserect;
    dst.w *= scale;
    dst.h *= scale;
    dst.x -= (dst.w - item.baserect.w) / 2;
    dst.y -= (dst.h - item.baserect.h) / 2;

    ApplyUIScale(uiScale, item.rect, dst);

    SDL_SetTextureAlphaMod(item.icon, alpha);
    SDL_RenderTexture(r, item.icon, nullptr, &item.rect);
    
}


void Menu_Init(Menu& game, SDL_Renderer* renderer, App* app) {
    auto basePath = GetBasePath();
    game.app=app;
    //logo image
    
    game.logoTex = LoadIcon(renderer,(basePath / "logo.png").string().c_str());
    //logo background
    game.logobgTex = LoadIcon(renderer,(basePath / "logobg.png").string().c_str());
    SDL_SetTextureAlphaMod(game.logobgTex, 100);

    // game.sidemenuitembgbaseRect={100,100,100,100};


    auto props = SDL_GetTextureProperties(game.logobgTex);
    game.logobaseRect.w = (float)SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_WIDTH_NUMBER, 0)/2;
    game.logobaseRect.h = (float)SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_HEIGHT_NUMBER, 0)/2;
    game.logobaseRect.x = (app->WIDTH - game.logobaseRect.w)/2;
    game.logobaseRect.y = 10;

    game.logobgbaseRect.x = game.logobaseRect.x - 20;
    game.logobgbaseRect.y = game.logobaseRect.y - 15;
    game.logobgbaseRect.w = game.logobaseRect.w + 40;
    game.logobgbaseRect.h = game.logobaseRect.h + 30;

    game.logoRect = game.logobaseRect;
    game.logobgRect = game.logobgbaseRect;
    const char* icons[] = {
    "menupic/tic_tac_toe.png",
    "menupic/chess.png",
    "menupic/memory.png",
    "menupic/seabattle.png",
    "menupic/snake_ladder.png",
    "menupic/yatzi.png",
    "menupic/dots_boxes.png",
    
};
    InitMenu(app,sizeof(icons)/sizeof(icons[0]));
    // InitSidePanel(app);
    //sidemenuitem background
    game.sidemenuitembgTex = LoadIcon(renderer,(basePath / "sidemenuitem.png").string().c_str());
    SDL_SetTextureAlphaMod(game.sidemenuitembgTex, 100);
    game.sidemenuitembgbaseRect={app->WIDTH-84*app->scale,472,313,664};
    game.sidemenuitembgRect=game.sidemenuitembgbaseRect;


for (int i = 0; i < app->menuItems.size(); i++) {
    app->menuItems[i].ui.icon = LoadIcon(app->renderer, (basePath / icons[i]).string().c_str());
}



}



void menu_switched(Menu& game){
    ApplyUIScale(game.app->scale,game.logoRect,game.logobaseRect);
    ApplyUIScale(game.app->scale,game.logobgRect,game.logobgbaseRect);
    game.sidemenuitembgRect.x=game.app->WIDTH-84*game.app->scale;
}
void Menu_HandleEvent(Menu& game, SDL_Event* e) {
    
    if (e->type == SDL_EVENT_WINDOW_RESIZED){
        menu_switched(game);
        // ApplyUIScale(game.app->scale,game.sidemenuitembgRect,game.sidemenuitembgbaseRect);
    }
    if (e->type == SDL_EVENT_MOUSE_BUTTON_DOWN &&e->button.button == SDL_BUTTON_LEFT &&game.app->hoveredIndex != -1) {
    switch (game.app->menuItems[game.app->hoveredIndex].type) {
    case MenuItemType::TicTacToe:
        game.app->state = GameState::TIC_TAC_TOE;
        break;
        }
    }
    

    if (e->type == SDL_EVENT_MOUSE_MOTION) {
        game.app->hoveredIndex = -1;

        for (int i = 0; i < game.app->menuItems.size(); i++) {
            auto& r = game.app->menuItems[i].ui.rect;
            if (game.app->mouseX >= r.x && game.app->mouseX <= r.x + r.w &&
                game.app->mouseY >= r.y && game.app->mouseY <= r.y + r.h) {
                game.app->hoveredIndex = i;
                
                break;
            }
        }
        for (int i = 0; i < game.app->sideButtons.size(); i++) {
        auto& r = game.app->sideButtons[i].ui.baserect;
    }
    }
    
    if (e->type == SDL_EVENT_KEY_DOWN) {
    // SDL_Log("pressed key : %s",SDL_GetKeyName(e->key.key));
    switch (e->key.key) {
    case SDLK_H:
        if (game.app->state == GameState::TIC_TAC_TOE){
        game.app->state = GameState::Menu;
        SDL_Log("Switched to Menu");
        }
        else if (game.app->state == GameState::Menu){
        game.app->state = GameState::TIC_TAC_TOE;
        SDL_Log("Switched to Tic Tac Toe");
        }
    }
    }
}


void Menu_Render(Menu& game, SDL_Renderer* renderer) {
    SDL_RenderTexture(renderer, game.logobgTex, nullptr, &game.logoRect);
    SDL_RenderTexture(renderer, game.logoTex, nullptr, &game.logoRect);
    SDL_RenderTexture(renderer, game.sidemenuitembgTex, nullptr, &game.sidemenuitembgRect);
    for (auto& item : game.app->menuItems) {
    UpdateHover(item.ui, game.app->mouseX, game.app->mouseY);
    RenderUIItem(game.app->renderer, item.ui, game.app->scale);

    if (item.ui.hovered) {
        SDL_FRect lr {
            item.ui.rect.x,
            item.ui.rect.y + item.ui.rect.h + 8,
            item.labelRect.w,
            item.labelRect.h
        };
        lr.x = item.ui.rect.x + (item.ui.rect.w - lr.w) / 2;
        SDL_RenderTexture(game.app->renderer, item.label, nullptr, &lr);
    }

}
    SidePanel_Render(game.app, renderer);

}

void Menu_Quit(Menu& game) {
    SDL_DestroyTexture(game.logoTex);
    SDL_DestroyTexture(game.logobgTex);
    SDL_DestroyTexture(game.sidemenuitembgTex);
    // SDL_Log("Menu_Quit");

}
