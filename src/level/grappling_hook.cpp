#include "grappling_hook.hpp"

#include "raylib.h"
#include "level.hpp"
#include "player.hpp"
#include "../linked_list.hpp"
#include "../camera.hpp"

#define ANGLE           PI/4
#define MAX_LENGTH      350
#define LAUNCH_ACCEL    0.3


GrapplingHook *GrapplingHook::Initialize() {

    GrapplingHook *hook = new GrapplingHook();

    hook->tags = Level::IS_HOOK;
    hook->isFacingRight = PLAYER_ENTITY->isFacingRight;

    LinkedList::AddNode(&Level::STATE->listHead, hook);

    TraceLog(LOG_TRACE, "Initialized grappling hook");

    return hook;
}

void GrapplingHook::Tick() {

    if (attachedTo) {
        // Do nothing, let the Player routine handle it
        return;
    }


    if (currentLength == MAX_LENGTH) {
        delete this;
        return;
    }


    static float currentSpeed = 0;
    currentLength += (currentSpeed += LAUNCH_ACCEL);
    if (currentLength > MAX_LENGTH) {
        currentLength = MAX_LENGTH;
        currentSpeed = 0;
    }


    float x = PLAYER_ENTITY->hitbox.x;
    if (isFacingRight) x += PLAYER_ENTITY->hitbox.width;

    float y = PLAYER_ENTITY->hitbox.y + (PLAYER_ENTITY->hitbox.height/3);
    
    this->start = { x, y };


    float endX;
    if (isFacingRight) endX = x + currentLength * cos(ANGLE); 
    else endX = x - currentLength * cos(ANGLE); 

    float endY = y - currentLength * sin(ANGLE);

    this->end = { endX, endY };


    Level::Entity *entity = (Level::Entity *) Level::STATE->listHead;
    while (entity) {

        if (entity->tags & Level::IS_HOOKABLE && CheckCollisionPointRec(this->end, entity->hitbox)) {
            attachedTo = entity;
            break;
        }

        entity = (Level::Entity *) entity->next;
    }
}

void GrapplingHook::Draw() {
    
    Vector2 posStart = PosInSceneToScreen(start);
    Vector2 posEnd = PosInSceneToScreen(end);
    DrawLine(posStart.x, posStart.y, posEnd.x, posEnd.y, WHITE);
}

GrapplingHook::~GrapplingHook() {

    LinkedList::RemoveNode(&Level::STATE->listHead, this);
    PLAYER_STATE->hookLaunched = 0;
    
    TraceLog(LOG_TRACE, "Destroying grappling hook");
}
