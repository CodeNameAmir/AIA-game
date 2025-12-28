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
void ApplyUIScale(float scale,SDL_FRect& item,const SDL_FRect& itembase) {
        item.x = itembase.x * scale;
        item.y = itembase.y * scale;
        item.w = itembase.w * scale;
        item.h = itembase.h * scale;
}


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

        // SDL_Log("%i",MenuItemNames[i]);
        // std::cout << MenuItemNames[i]<< std::endl;
        const std::string_view text = MenuItemNames[i];
        SDL_Surface* msgSurf = TTF_RenderText_Solid(font, text.data(), text.length(), {105,105,105,255});
        app->menuItems[i].label = SDL_CreateTextureFromSurface(app->renderer, msgSurf);
        SDL_DestroySurface(msgSurf);
        auto props = SDL_GetTextureProperties(app->menuItems[i].label);
        app->menuItems[i].labelRect.w = (float)SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_WIDTH_NUMBER, 0);
        app->menuItems[i].labelRect.h = (float)SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_HEIGHT_NUMBER, 0);
    }
    
}
SDL_Texture* LoadIcon(SDL_Renderer* r, const std::string& path) {
    SDL_Surface* s = IMG_Load(path.c_str());
    if (!s) {
        SDL_Log("Failed to load %s", path.c_str());
        return nullptr;
    }
    SDL_Texture* t = SDL_CreateTextureFromSurface(r, s);
    SDL_DestroySurface(s);
    return t;
}

void InitSidePanel(App* app)
{
    float gap = 20.0f;
    float size = 56.0f;

    float rightMargin = 20.0f;   // فاصله از راست
    float bottomMargin = 20.0f;  // فاصله از پایین

    float panelX = 1280 - rightMargin - size;
    float startY = 720 - bottomMargin - (size * 3 + gap * 2);
    
    for (int i = 0; i < 3; i++) {
        SideButton b{};
        b.ui.baserect = {
            panelX,
            startY + i * (size + gap),
            size,
            size
        };
        b.ui.spacey=app->HEIGHT-b.ui.baserect.y;
        app->sideButtons.push_back(b);
    }
    auto basePath = GetBasePath();
    app->sideButtons[0].ui.icon = LoadIcon(app->renderer, (basePath / "icons/about.png").string().c_str());
    app->sideButtons[1].ui.icon = LoadIcon(app->renderer, (basePath / "icons/setting.png").string().c_str());
    app->sideButtons[2].ui.icon = LoadIcon(app->renderer, (basePath / "icons/exit.png").string().c_str());
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
void RendersidepanelItem(SDL_Renderer* r, UIItem& item, float uiScale,App* app,Menu& game) {
    float scale = item.hovered ? item.hoverScale : 1.0f;
    Uint8 alpha = item.hovered ? item.hoverAlpha : item.normalAlpha;

    SDL_FRect dst = item.baserect;
    dst.w *= scale;
    dst.h *= scale;
    dst.x -= (dst.w - item.baserect.w) / 2;
    dst.y -= (dst.h - item.baserect.h) / 2;

    ApplyUIScale(uiScale, item.rect, dst);
    item.rect.x=app->WIDTH-70*app->scale;
    item.rect.y = app->HEIGHT-item.spacey*app->scale;
    game.sidemenuitembgRect.y=game.app->sideButtons[0].ui.rect.y-20;

    SDL_SetTextureAlphaMod(item.icon, alpha);
    SDL_RenderTexture(r, item.icon, nullptr, &item.rect);
    
}

void Menu_Init(Menu& game, SDL_Renderer* renderer, App* app) {
    auto basePath = GetBasePath();
    game.app=app;
    //logo image
    SDL_Surface* logoSurf = IMG_Load((basePath / "logo.png").string().c_str());
    if (!logoSurf) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: %s", "Load logo.png", SDL_GetError());;
    game.logoTex = SDL_CreateTextureFromSurface(renderer, logoSurf);
    SDL_DestroySurface(logoSurf);
    //logo background
    SDL_Surface* logobgSurf = IMG_Load((basePath / "logobg.png").string().c_str());
    if (!logobgSurf) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: %s", "Load logobg.png", SDL_GetError());;
    game.logobgTex = SDL_CreateTextureFromSurface(renderer, logobgSurf);
    SDL_SetTextureAlphaMod(game.logobgTex, 100);
    SDL_DestroySurface(logobgSurf);
    
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
    "icons/tic_tac_toe.png",
    "icons/chess.png",
    "icons/memory.png",
    "icons/seabattle.png",
    "icons/snake_ladder.png",
    "icons/yatzi.png",
    "icons/dots_boxes.png",
    
};
    InitMenu(app,sizeof(icons)/sizeof(icons[0]));
    InitSidePanel(app);
    //sidemenuitem background
    SDL_Surface* sidemenuitembgSurf = IMG_Load((basePath / "icons/sidemenuitem.png").string().c_str());
    if (!sidemenuitembgSurf) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: %s", "Load sidemenuitem.png", SDL_GetError());;
    game.sidemenuitembgTex = SDL_CreateTextureFromSurface(renderer, sidemenuitembgSurf);
    SDL_SetTextureAlphaMod(game.sidemenuitembgTex, 100);
    SDL_DestroySurface(sidemenuitembgSurf);
    game.sidemenuitembgbaseRect={app->WIDTH-84*app->scale,app->sideButtons[0].ui.baserect.y-20,313,664};
    game.sidemenuitembgRect=game.sidemenuitembgbaseRect;


for (int i = 0; i < app->menuItems.size(); i++) {
    app->menuItems[i].ui.icon = LoadIcon(app->renderer, (basePath / icons[i]).string().c_str());
}



}

bool PointInRect(float mx, float my, const SDL_FRect& r) {
    return mx >= r.x && mx <= r.x + r.w &&
           my >= r.y && my <= r.y + r.h;
}
void UpdateHover(UIItem& item, float mx, float my) {
    item.hovered = PointInRect(mx, my, item.rect);
}


void Menu_HandleEvent(Menu& game, SDL_Event* e) {
    
    if (e->type == SDL_EVENT_WINDOW_RESIZED){
        ApplyUIScale(game.app->scale,game.logoRect,game.logobaseRect);
        ApplyUIScale(game.app->scale,game.logobgRect,game.logobgbaseRect);
        game.sidemenuitembgRect.x=game.app->WIDTH-84*game.app->scale;

        // ApplyUIScale(game.app->scale,game.sidemenuitembgRect,game.sidemenuitembgbaseRect);
    }
    if (e->type == SDL_EVENT_MOUSE_BUTTON_DOWN &&e->button.button == SDL_BUTTON_LEFT &&game.app->hoveredIndex != -1) {
    switch (game.app->menuItems[game.app->hoveredIndex].type) {
    case MenuItemType::TicTacToe:
        game.app->state = GameState::TIC_TAC_TOE;
        break;
        }
    }
    if (e->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
    for (int i = 0; i < game.app->sideButtons.size(); i++) {
        auto& r = game.app->sideButtons[i].ui.rect;
        if (game.app->mouseX >= r.x && game.app->mouseX <= r.x + r.w &&
            game.app->mouseY >= r.y && game.app->mouseY <= r.y + r.h) {
            if (i == 0) SDL_Log("about");
            if (i == 1) SDL_Log("setting");
            if (i == 2) game.app->quitState = SDL_APP_SUCCESS;
        }
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
        if (game.app->mouseX >= r.x && game.app->mouseX <= r.x + r.w &&
            game.app->mouseY >= r.y && game.app->mouseY <= r.y + r.h) {
            if (i == 0) SDL_Log("about");
            if (i == 1) SDL_Log("setting");
            if (i == 2) SDL_Log("exit");
        }
    }
    }
    
    if (e->type == SDL_EVENT_KEY_DOWN) {
    SDL_Log("pressed key : %s",SDL_GetKeyName(e->key.key));
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
        SDL_FRect lr {
            item.ui.rect.x,
            item.ui.rect.y + item.ui.rect.h + 8,
            item.labelRect.w,
            item.labelRect.h
        };
        lr.x = item.ui.rect.x + (item.ui.rect.w - lr.w) / 2;
        SDL_RenderTexture(game.app->renderer, item.label, nullptr, &lr);
    }
    // game.sidemenuitembgRect=game.app->sideButtons[0].ui.rect;
    for (auto& b : game.app->sideButtons) {
    UpdateHover(b.ui, game.app->mouseX, game.app->mouseY);
    RendersidepanelItem(renderer, b.ui, game.app->scale,game.app,game.app->menu);
    // std::cout << b.ui.rect.x<<" x "<<b.ui.rect.y<<"\t"<<b.ui.rect.w<<" x "<<b.ui.rect.h << std::endl;
    
}

}


}

void Menu_Quit(Menu& game) {
    SDL_DestroyTexture(game.logoTex);
    SDL_DestroyTexture(game.logobgTex);
    SDL_DestroyTexture(game.sidemenuitembgTex);
    // SDL_Log("Menu_Quit");

}
