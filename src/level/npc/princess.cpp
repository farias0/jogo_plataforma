#include "princess.hpp"
#include "../../render.hpp"


Princess *Princess::Add() {
    return Add({ 0, 0 });
}

Princess *Princess::Add(Vector2 pos) {
    
    Princess *newPrincess = new Princess();

    Sprite *sprite = &SPRITES->PrincessDefault1;
    Rectangle hitbox = SpriteHitboxFromEdge(sprite, pos);

    newPrincess->tags = Level::IS_NPC +
                            Level::IS_PERSISTABLE;
    newPrincess->hitbox = hitbox;
    newPrincess->origin = pos;
    newPrincess->sprite = sprite;
    newPrincess->layer = -1;
    newPrincess->isFacingRight = true;

    newPrincess->isFalling = true;

    LinkedList::AddNode(&Level::STATE->listHead, newPrincess);

    TraceLog(LOG_TRACE, "Added princess to level (x=%.1f, y=%.1f)",
                newPrincess->hitbox.x, newPrincess->hitbox.y);

    return newPrincess;
}

void Princess::CheckAndAdd(Vector2 pos) {

    Rectangle hitbox = SpriteHitboxFromMiddle(&SPRITES->PrincessDefault1, pos);

    if (Level::CheckCollisionWithAnything(hitbox)) {
        Render::PrintSysMessage("Sem espaço para NPC (Princesa)");
        TraceLog(LOG_DEBUG, "Couldn't add Princess to level, collision with entity.");
        return;
    }

    Add({ hitbox.x, hitbox.y });
}
