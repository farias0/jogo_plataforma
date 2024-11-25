#pragma once


#include <string>

#include "level.hpp"
#include "../text_bank.hpp"
#include "../animation.hpp"


#define TEXTBOX_BUTTON_ENTITY_ID       "textbox_button"


class Textbox : public Level::Entity, private Animation::IAnimated {

public:

    int textId;
    std::string textContent;
    bool isDevTextbox;

    // The textbox being currently displayed
    static Textbox *TextboxDisplaying;


    ~Textbox();

    static Textbox *AddFromPersistence();

    // Initializes and adds a textbox button to the level
    static Textbox *Add(Vector2 pos, int textId);

    // Initializes and adds a a textbox button to the level in the
    // given origin, if there are no other entities there already
    static void AddFromEditor(Vector2 pos, int interactionTags);

    void SetTextId(int textId);

    // Toggles between normal and dev textboxes
    void ToggleTextboxType();

    // Toggles the exhibition of this textbox
    void Toggle();

    // Reloads the text in all the texboxes part of the loaded level from disk
    static void ReloadAllLevelTexboxes();

    std::string GetEntityDebugString() override;

    void Tick() override;

    void Draw() override;

    void PersistenceParse(const std::string &data) override;
    std::string PersistanceSerialize() override;

private:

    static Animation::Animation animationOff;
    static Animation::Animation animationOffDev;
    static Animation::Animation animationPlaying;


    static void createFromIdInput(Vector2 pos, std::string input);

    // Sets the correct sprite to a textbox accordingly to its type 
    void updateSprite();

    void createAnimations();
    
    Animation::Animation *getCurrentAnimation();
};
