#include <functional>
#include <stdexcept>

#include "textbox.hpp"
#include "level.hpp"
#include "../input.hpp"
#include "../camera.hpp"
#include "../core.hpp"
#include "../editor.hpp"


#define TEXT_NOT_FOUND_CONTENT      "ERRO: Texto não encontrado!"


Textbox *Textbox::TextboxDisplaying = 0;

Textbox::~Textbox() {
    if (TextboxDisplaying == this) TextboxDisplaying = 0;
}

Textbox *Textbox::Add() {
    return Add({ 0, 0 }, -1);
}

Textbox *Textbox::Add(Vector2 pos, int textId) {

    Textbox *newTextbox = new Textbox();

    Sprite *sprite = &SPRITES->TextboxButton;
    Rectangle hitbox = SpriteHitboxFromEdge(sprite, pos);

    newTextbox->tags = Level::IS_TEXTBOX +
                        Level::IS_PERSISTABLE;
    newTextbox->hitbox = hitbox;
    newTextbox->origin = pos;
    newTextbox->sprite = sprite;
    newTextbox->layer = -1;
    newTextbox->isFacingRight = true;
    newTextbox->isDevTextbox = false;
    newTextbox->SetTextId(textId);

    LinkedList::AddNode(&Level::STATE->listHead, newTextbox);

    TraceLog(LOG_TRACE, "Added textbox button to level (x=%.1f, y=%.1f)",
                newTextbox->hitbox.x, newTextbox->hitbox.y);

    return newTextbox;
}

void Textbox::CheckAndAdd(Vector2 pos, int interactionTags) {

    if (interactionTags & EDITOR_INTERACTION_HOLD) return;


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

void Textbox::SetTextId(int textId) {

    this->textId = textId;

    std::string queryText = TextBank::BANK[textId];
    if (queryText.length()) {
        textContent = queryText;
    } else {
        textContent = std::string(TEXT_NOT_FOUND_CONTENT);
    }
}

void Textbox::ToggleTextboxType() {
    isDevTextbox = !isDevTextbox;
    updateSprite();
}

void Textbox::Toggle() {
    if (TextboxDisplaying == this) TextboxDisplaying = 0;
    else TextboxDisplaying = this;
}

void Textbox::ReloadAllLevelTexboxes() {
    
    for (Level::Entity *e = (Level::Entity *) Level::STATE->listHead;
        e != 0;
        e = (Level::Entity *) e->next) {

            if (e->tags & Level::IS_TEXTBOX) {
                auto box = (Textbox *) e;
                box->SetTextId(box->textId);
            }
    }
}

std::string Textbox::GetEntityDebugString() {
    return Level::Entity::GetEntityDebugString() +
            "\ntextId=" + std::to_string(textId);
}

void Textbox::Draw() {
        
    if (isDevTextbox && !IsDevTextboxEnabled() && !EDITOR_STATE->isEnabled)
        return;

    // Draw the interactable button 
    Level::Entity::Draw();
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
    int id = std::stoi(persistenceReadValue(data, "textId"));
    SetTextId(id);
    isDevTextbox = (bool) std::stoi(persistenceReadValue(data, "isDevTextbox"));

    updateSprite();
}
