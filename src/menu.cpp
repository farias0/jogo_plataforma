#include <string>

#include "menu.hpp"
#include "core.hpp"
#include "level/player.hpp"


#define MENU_ORIGIN             (float)GetScreenWidth()/2-70, 360

#define HEADER_FONT_SIZE        30
#define LABEL_FONT_SIZE         20

#define SPACING_HEADER_BODY     100
#define SPACING_LABELS          50


void MenuItem::Select() {
    callback();
}

void MenuItem::Draw(Vector2 pos) {
    DrawText(label.c_str(), pos.x, pos.y, LABEL_FONT_SIZE, WHITE);
}

//

void MenuItemToggle::Draw(Vector2 pos) {

    std::string str = "[";
    if (isToggledMonitor()) str += "x";
    else str += " ";
    str += "] " + label;

    DrawText(str.c_str(), pos.x, pos.y, LABEL_FONT_SIZE, WHITE);
}

//

Menu::~Menu() {
    for (auto item : items) {
        delete item;
    }
}

void Menu::AddItem(MenuItem* item) {
    items.push_back(item);
}

void Menu::Select() {
    items.at(itemHighlighted)->Select();
}

void Menu::Up() {
    itemHighlighted--;
    if (itemHighlighted == -1) itemHighlighted = items.size() - 1;
}

void Menu::Down() {
    itemHighlighted++;
    if (itemHighlighted == (int) items.size()) itemHighlighted = 0;
}

void Menu::Draw() {

    Vector2 pos = { MENU_ORIGIN };

    // Darken background
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), { 0x00, 0x00, 0x00, 0xaa });

    // TODO fix pause state (let pause in ow, don't let pause if there's no level loaded)
    // and set this to always show
    if (Level::STATE->isPaused && PLAYER && !PLAYER->isDead)
        DrawText("PAUSADO", pos.x, pos.y, HEADER_FONT_SIZE, RAYWHITE);

    pos.y += SPACING_HEADER_BODY;
    int itemCount = 0;

    for (auto item : items) {

        item->Draw(pos);

        if (itemCount == itemHighlighted) {
            DrawText("=>", pos.x - 45, pos.y, LABEL_FONT_SIZE, WHITE);
        }

        pos.y += SPACING_LABELS;
        itemCount++;
    }
}
