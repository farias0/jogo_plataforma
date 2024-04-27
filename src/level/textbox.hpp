#pragma once

#include "level.hpp"


class Textbox : public Level::Entity {

public:
    int textId;

    // Initializes and adds a textbox button to the level
    static void Add(Vector2 pos, int textId);

    // Initializes and adds a a textbox button to the level in the
    // given origin, if there are no other entities there already
    static void CheckAndAdd(Vector2 pos);

private:
    static void createFromIdInput(Vector2 pos, std::string input);

};
