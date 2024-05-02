#include <functional>
#include <stdexcept>

#include "textbox.hpp"
#include "../input.hpp"


Textbox *Textbox::Add() {
    return Add({ 0, 0 }, -1);
}

Textbox *Textbox::Add(Vector2 pos, int textId) {

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
    newTextbox->isDevTextbox = false;

    LinkedList::AddNode(&Level::STATE->listHead, newTextbox);

    TraceLog(LOG_TRACE, "Added textbox button to level (x=%.1f, y=%.1f)",
                newTextbox->hitbox.x, newTextbox->hitbox.y);

    return newTextbox;
}

void Textbox::CheckAndAdd(Vector2 pos) {

    Rectangle hitbox = SpriteHitboxFromMiddle(&SPRITES->TextboxButton, pos);

    Level::Entity *entityCollidedWith = Level::CheckCollisionWithAnything(hitbox); 
    if (entityCollidedWith) {
        if (entityCollidedWith->tags & Level::IS_TEXTBOX) {
            auto box = (Textbox *) entityCollidedWith;
            box->ToggleTextboxType();
            if (box->isDevTextbox) Render::PrintSysMessage("Caixa de texto do desenvolvedor ativa");
        }
        else TraceLog(LOG_DEBUG, "Couldn't add textbox button, collision with entity.");
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

void Textbox::ToggleTextboxType() {
    isDevTextbox = !isDevTextbox;
    updateSprite();
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

void Textbox::updateSprite() {
    if (isDevTextbox) {
        sprite = &SPRITES->TextboxDevButton;
    } else {
        sprite = &SPRITES->TextboxButton;
    }
}

std::string Textbox::PersistanceSerialize() {

    std::string data = Level::Entity::PersistanceSerialize();
    persistanceAddValue(&data, "textId", std::to_string(textId));
    persistanceAddValue(&data, "isDevTextbox", std::to_string((int) isDevTextbox));
    return data;
}

void Textbox::PersistenceParse(const std::string &data) {

    Level::Entity::PersistenceParse(data);
    textId = std::stoi(persistenceReadValue(data, "textId"));
    isDevTextbox = (bool) std::stoi(persistenceReadValue(data, "isDevTextbox"));

    updateSprite();
}
