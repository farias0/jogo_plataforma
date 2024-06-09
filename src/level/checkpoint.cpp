#include "checkpoint.hpp"
#include "level.hpp"
#include "../animation.hpp"
#include "../editor.hpp"

#define ANIMATION_DURATION_STILL    180
#define ANIMATION_DURATION_SHAKING  5



Animation::Animation CheckpointPickup::animation;


Level::Entity *CheckpointPickup::Add() {
    return Add({ 0, 0 });
}

Level::Entity *CheckpointPickup::Add(Vector2 pos) {

    CheckpointPickup *newPickup = new CheckpointPickup();

    Sprite *sprite = &SPRITES->LevelCheckpointPickup1;
    Rectangle hitbox = SpriteHitboxFromEdge(sprite, pos);

    newPickup->tags = Level::IS_CHECKPOINT_PICKUP +
                        Level::IS_PERSISTABLE;
    newPickup->hitbox = hitbox;
    newPickup->origin = pos;
    newPickup->sprite = sprite;
    newPickup->isFacingRight = true;
    newPickup->layer = -1;
    newPickup->entityTypeID = CHECKPOINT_PICKUP_ENTITY_ID;

    newPickup->wasPickedUp = false;

    newPickup->initializeAnimationSystem();

    LinkedList::AddNode(&Level::STATE->listHead, newPickup);

    TraceLog(LOG_TRACE, "Added checkpoint pickup to level (x=%.1f, y=%.1f)",
                newPickup->hitbox.x, newPickup->hitbox.y);

    return newPickup;
}

void CheckpointPickup::CheckAndAdd(Vector2 pos, int interactionTags) {

    if (!(interactionTags & EDITOR_INTERACTION_CLICK)) return;
    

    Rectangle hitbox = SpriteHitboxFromMiddle(&SPRITES->LevelCheckpointPickup1, pos);

    if (Level::CheckCollisionWithAnything(hitbox)) {
        TraceLog(LOG_DEBUG, "Couldn't add checkpoint pickup, collision with entity.");
        return;
    }
    
    Add({ hitbox.x, hitbox.y });
}

void CheckpointPickup::Tick() {

    sprite = animationTick();
}

void CheckpointPickup::Draw() {

    if (!wasPickedUp) {
        Render::DrawLevelEntity(this);   
    }
    else if (EDITOR_STATE->isEnabled) {
        Render::DrawLevelEntityOriginGhost(this);
    }
}

void CheckpointPickup::createAnimations() {

    animation.AddFrame(&SPRITES->LevelCheckpointPickup2, ANIMATION_DURATION_SHAKING);
    animation.AddFrame(&SPRITES->LevelCheckpointPickup1, ANIMATION_DURATION_SHAKING);
    animation.AddFrame(&SPRITES->LevelCheckpointPickup3, ANIMATION_DURATION_SHAKING);
    animation.AddFrame(&SPRITES->LevelCheckpointPickup1, ANIMATION_DURATION_STILL);
}

Animation::Animation *CheckpointPickup::getCurrentAnimation() {

    return &animation;
}
