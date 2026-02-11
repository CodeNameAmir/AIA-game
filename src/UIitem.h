enum class MenuItemType {
    TicTacToe,
    SeaBattle,
    Yahtzee,
    Memory,
    Chess,
    Snakes,
    DotsAndBoxes
};
struct UIItem {
    SDL_FRect baserect;
    SDL_FRect rect;
    SDL_Texture* icon = nullptr;

    bool hovered = false;
    float hoverScale = 1.1f;
    float spacex=0.0f;
    float spacey=0.0f;
    Uint8 normalAlpha = 180;
    Uint8 hoverAlpha = 255;
};

struct SideButton {
    UIItem ui;
};


struct MenuItem {
    MenuItemType type;
    UIItem ui;

    SDL_Texture* label = nullptr;
    SDL_FRect labelRect;
};