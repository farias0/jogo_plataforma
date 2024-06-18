#include "coin.hpp"

Coin *Coin::Add() {
    return Add({ 0,0 });
}

Coin *Coin::Add(Vector2 origin) {

    Coin *newCoin = new Coin();

    newCoin->tags = Level::IS_PERSISTABLE;
    newCoin->origin = origin;
    newCoin->sprite = &SPRITES->Coin1;	
    newCoin->hitbox = SpriteHitboxFromEdge(newCoin->sprite, newCoin->origin);
    newCoin->entityTypeID = COIN_ENTITY_ID;
    newCoin->isFacingRight = true;

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

std::string Coin::PersistanceSerialize() {
    std::string data = Level::Entity::PersistanceSerialize();
    return data;
}

void Coin::PersistenceParse(const std::string &data) {
    Level::Entity::PersistenceParse(data);
}
