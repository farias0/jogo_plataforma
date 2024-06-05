#include "princess.hpp"
#include "../../editor.hpp"


Princess *Princess::Add() {
    return Add({ 0, 0 });
}

Princess *Princess::Add(Vector2 pos) {
    
    Princess *newPrincess = new Princess();

    Sprite *sprite = &SPRITES->PrincessDefault1;
    Rectangle hitbox = SpriteHitboxFromEdge(sprite, pos);

    newPrincess->tags = Level::IS_PERSISTABLE;
    newPrincess->hitbox = hitbox;
    newPrincess->origin = pos;
    newPrincess->sprite = sprite;
    newPrincess->layer = -1;
    newPrincess->isFacingRight = true;

    newPrincess->isFalling = true;

    LinkedList::AddNode(&Level::STATE->listHead, newPrincess);

    TraceLog(LOG_TRACE, "Added npc to level (x=%.1f, y=%.1f)",
                newPrincess->hitbox.x, newPrincess->hitbox.y);

    return newPrincess;
}

void Princess::CheckAndAdd(Vector2 pos, int interactionTags) {

    if (!(interactionTags & EDITOR_INTERACTION_CLICK)) return;

    // TODO check

    Add(pos);
}
