#pragma once

#include <string>
#include <vector>

#include <raylib.h>

#include "render.hpp"


#define MENU_ORIGIN             GetScreenWidth()/2-70, 360

#define HEADER_FONT_SIZE        30
#define LABEL_FONT_SIZE         20

#define SPACING_HEADER_BODY     100
#define SPACING_LABELS          50


// TODO create menu.cpp


class MenuItem {

public:

    // The text that shows as the option
    std::string label;

    MenuItem(std::string label, void (*callback)()) :
                label(label), callback(callback) {}

    void Select() {
        callback();
    }

    virtual void Draw(Vector2 pos) {
        DrawText(label.c_str(), pos.x, pos.y, LABEL_FONT_SIZE, WHITE);
    }

private:

    // Function to be executed when item is selected
    void (*callback)();

};

class MenuItemToggle : public MenuItem {

public:
    MenuItemToggle(std::string label, void (*callback)(), bool (*isToggledMonitor)()) :
                        MenuItem(label, callback), isToggledMonitor(isToggledMonitor) {}

    void Draw(Vector2 pos) override {

        std::string str = "[";
        if (isToggledMonitor()) str += "x";
        else str += " ";
        str += "] " + label;

        DrawText(str.c_str(), pos.x, pos.y, LABEL_FONT_SIZE, WHITE);
    }

private:

    // Bool-returning function to control if option is shown as toggled
    bool (*isToggledMonitor)();

};

class Menu {

public:

    std::vector<MenuItem*> items;
    int itemHighlighted;


    ~Menu() {
        for (auto item : items) {
            delete item;
        }
    }

    void AddItem(MenuItem* item) {
        items.push_back(item);
    }

    void Select() {
        items.at(itemHighlighted)->Select();
    }

    void Up() {
        itemHighlighted--;
        if (itemHighlighted == -1) itemHighlighted = items.size() - 1;
    }

    void Down() {
        itemHighlighted++;
        if (itemHighlighted == items.size()) itemHighlighted = 0;
    }

    void Draw() {

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
};
