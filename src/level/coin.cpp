#include <stdlib.h>

#include "coin.hpp"
#include "../debug.hpp"
#include "../editor.hpp"


#define COIN_ANIMATION_BLINK_PERIOD  8 // in framees


Animation::Animation Coin::animationIdle;
Animation::Animation Coin::animationBlinking;


Coin *Coin::Add() {
    return Add({ 0,0 });
}

Coin *Coin::Add(Vector2 origin) {

    Coin *newCoin = new Coin();

    newCoin->tags = Level::IS_COIN +
                        Level::IS_PERSISTABLE;
    newCoin->origin = origin;
    newCoin->sprite = &SPRITES->Coin1;	
    newCoin->hitbox = SpriteHitboxFromEdge(newCoin->sprite, newCoin->origin);
    newCoin->entityTypeID = COIN_ENTITY_ID;
    newCoin->isFacingRight = true;
    newCoin->idlePeriod = rand() % 360 + 240;

    newCoin->initializeAnimationSystem();

    LinkedList::AddNode(&Level::STATE->listHead, newCoin);

    TraceLog(LOG_TRACE, "Added coin to level (x=%.1f, y=%.1f)",
                newCoin->hitbox.x, newCoin->hitbox.y);

    return newCoin;
}

void Coin::AddFromEditor(Vector2 origin, int interactionTags) {

    (void)interactionTags;

    Rectangle hitbox = SpriteHitboxFromEdge(&SPRITES->Coin1, origin);
    if (Level::CheckCollisionWithAnything(hitbox)) return;
    
    Add(origin);
}

void Coin::PickUp() {

    wasPickedUp = true;

    GAME_STATE->coinsCollected++;

    DebugEntityStop(this);

    TraceLog(LOG_TRACE, "Picked up coin.");
}

bool Coin::IsDisabled() {
    return wasPickedUp;
}

void Coin::Tick() {

    timeIntoIdle++;
    if (timeIntoIdle > idlePeriod + COIN_ANIMATION_BLINK_PERIOD) timeIntoIdle = 0;

    sprite = animationTick();
}

void Coin::Draw() {

    if (!wasPickedUp)
        Render::DrawLevelEntity(this);

    if (EDITOR_STATE->isEnabled)
        Render::DrawLevelEntityOriginGhost(this);
}

std::string Coin::PersistanceSerialize() {
    std::string data = Level::Entity::PersistanceSerialize();
    return data;
}

void Coin::PersistenceParse(const std::string &data) {
    Level::Entity::PersistenceParse(data);
}

void Coin::createAnimations() {

    animationIdle.AddFrame(&SPRITES->Coin1, 1);

    animationBlinking.AddFrame(&SPRITES->Coin2, COIN_ANIMATION_BLINK_PERIOD / 4);
    animationBlinking.AddFrame(&SPRITES->Coin3, COIN_ANIMATION_BLINK_PERIOD / 2);
    animationBlinking.AddFrame(&SPRITES->Coin2, COIN_ANIMATION_BLINK_PERIOD / 4);
}


Animation::Animation * Coin::getCurrentAnimation() {

    if (timeIntoIdle < idlePeriod) {
        return &animationIdle;
    } else {
        return &animationBlinking;
    }
}
