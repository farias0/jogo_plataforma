#include <functional>
#include <stdexcept>

#include "textbox.hpp"
#include "../input.hpp"


void Textbox::Add(Vector2 pos, int textId) {

    Textbox *newTextbox = new Textbox();

    Sprite *sprite = &SPRITES->TextboxButton;
    Rectangle hitbox = SpriteHitboxFromEdge(sprite, pos);

    newTextbox->tags = Level::IS_TEXTBOX;
    newTextbox->hitbox = hitbox;
    newTextbox->origin = pos;
    newTextbox->sprite = sprite;
    newTextbox->layer = -1;
    newTextbox->isFacingRight = true;
    newTextbox->textId = textId;

    LinkedList::AddNode(&Level::STATE->listHead, newTextbox);

    TraceLog(LOG_TRACE, "Added textbox button to level (x=%.1f, y=%.1f)",
                newTextbox->hitbox.x, newTextbox->hitbox.y);
}

void Textbox::CheckAndAdd(Vector2 pos) {

    Rectangle hitbox = SpriteHitboxFromMiddle(&SPRITES->TextboxButton, pos);

    if (Level::CheckCollisionWithAnything(hitbox)) {
        TraceLog(LOG_DEBUG, "Couldn't add textbox button, collision with entity.");
        return;
    }

    pos = RectangleGetPos(hitbox);
    TextInputCallback *callback = new TextInputCallback(
        [pos] (std::string input) { 
            createFromIdInput(pos, input);
        }
    );

    Render::PrintSysMessage("Insira o ID do texto");
    Input::GetTextInput(callback);
}

void Textbox::createFromIdInput(Vector2 pos, std::string input) {

    int id;
    
    try {
        id = std::stoi(input);
    }
    catch (std::invalid_argument &e) {
        Render::PrintSysMessage("ID inválido");
        TraceLog(LOG_DEBUG, "Textbox ID input invalid: %s.", input.c_str());
        return; // does nothing
    }

    Add(pos, id);
}

