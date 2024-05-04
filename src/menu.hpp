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


class MenuItem {

public:

    // The text that shows as the option
    std::string label;

    MenuItem(std::string label, void (*callback)()) :
                label(label), callback(callback) {}

    void Select();

    virtual void Draw(Vector2 pos);

private:

    // Function to be executed when item is selected
    void (*callback)();

};


class MenuItemToggle : public MenuItem {

public:
    MenuItemToggle(std::string label, void (*callback)(), bool (*isToggledMonitor)()) :
                        MenuItem(label, callback), isToggledMonitor(isToggledMonitor) {}

    void Draw(Vector2 pos) override;

private:

    // Bool-returning function to control if option is shown as toggled
    bool (*isToggledMonitor)();

};


class Menu {

public:

    ~Menu();

    void AddItem(MenuItem* item);

    void Select();

    void Up();

    void Down();

    void Draw();

private:

    std::vector<MenuItem*> items;

    // Index of the menu item that is highlighted
    int itemHighlighted;

};
