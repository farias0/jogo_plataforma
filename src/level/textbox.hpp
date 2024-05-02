#pragma once


#include <string>

#include "level.hpp"
#include "../text_bank.hpp"


#define TEXTBOX_BUTTON_PERSISTENCE_ID       "textbox_button"


class Textbox : public Level::Entity {

public:

    int textId;
    std::string textContent;
    bool isDevTextbox;


    ~Textbox();

    static Textbox *Add();

    // Initializes and adds a textbox button to the level
    static Textbox *Add(Vector2 pos, int textId);

    // Initializes and adds a a textbox button to the level in the
    // given origin, if there are no other entities there already
    static void CheckAndAdd(Vector2 pos);

    // Toggles between normal and dev textboxes
    void ToggleTextboxType();

    // Toggles the exhibition of this textbox
    void Toggle();

    void Draw() override;

    void PersistenceParse(const std::string &data) override;
    std::string PersistanceSerialize() override;

    std::string PersistanceEntityID() {
        return TEXTBOX_BUTTON_PERSISTENCE_ID;
    }

private:

    // The textbox being currently displayed
    static Textbox *textboxDisplaying;


    static void createFromIdInput(Vector2 pos, std::string input);

    // Sets the correct sprite to a textbox accordingly to its type 
    void updateSprite();
};
