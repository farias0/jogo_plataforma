#include "grappling_hook.hpp"

#include "raylib.h"
#include <cmath>

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include "raymath.h"
#pragma GCC diagnostic pop

#include "level.hpp"
#include "player.hpp"
#include "../linked_list.hpp"
#include "../camera.hpp"

#define ANGLE           PI/3 // With the end being y0 and start being y, 0 <= ANGLE < PI/2
#define MAX_LENGTH      400
#define THICKNESS       2.0f
#define LAUNCH_ACCEL    1
 
// Swinging constants
#define GRAVITY_ACCEL               12
#define SWINGING_TIME_STEP          0.1
#define ANGULAR_VELOCITY_INITIAL    0.24
#define DAMPING_FACTOR              0.02




GrapplingHook *GrapplingHook::Initialize() {

    GrapplingHook *hook = new GrapplingHook();

    hook->tags = Level::IS_HOOK;
    hook->isFacingRight = PLAYER_ENTITY->isFacingRight;

    hook->UpdateStartPos();
    hook->end = hook->start;

    if (hook->isFacingRight) hook->currentAngle = PI + ANGLE;
    else hook->currentAngle = 2*PI - ANGLE;

    LinkedList::AddNode(&Level::STATE->listHead, hook);

    TraceLog(LOG_TRACE, "Initialized grappling hook");

    return hook;
}

void GrapplingHook::Tick() {

    if (attachedTo) {
        // Do nothing, let the Player routine handle it
        return;
    }


    if (currentLength >= MAX_LENGTH) { // didn't attach
        delete this;
        return;
    }


    static float currentSpeed = 0;
    currentLength += (currentSpeed += LAUNCH_ACCEL);
    if (currentLength > MAX_LENGTH) {
        currentLength = MAX_LENGTH;
        currentSpeed = 0;
    }


    UpdateStartPos(); // hook follows player while not attached


    // calculate hook's end based on start + angle

    Vector2 projectedEnd;
    if (isFacingRight) projectedEnd.x = this->start.x + currentLength * cos(currentAngle - PI); 
    else projectedEnd.x = this->start.x - currentLength * cos(currentAngle); 
    projectedEnd.y = this->start.y + currentLength * sin(currentAngle);


    /*
        If a collision against an IS_HOOKABLE is checked just for the new 'end' coord, an entity that's
        in the hook's path might not collide with 'end' because 'end' traversed it in between frames.

        This is by remedied by also checking for various inbetween points between last frame's and this frame's 'end',
        which lowers the chance of this happening but still has the same fundamental flaw.
    */

    const Vector2 projectedEndPartway2 = { (this->end.x + projectedEnd.x)/2, (this->end.y + projectedEnd.y)/2};
    const Vector2 projectedEndPartway1 = { (this->end.x + projectedEndPartway2.x)/2, (this->end.y + projectedEndPartway2.y)/2 };
    const Vector2 projectedEndPartway3 = { (projectedEndPartway2.x + projectedEnd.x)/2, (projectedEndPartway2.y + projectedEnd.y)/2 };


    this->end = projectedEnd; // In case of collision the end should be different


    Level::Entity *entity = (Level::Entity *) Level::STATE->listHead;
    while (entity) {

        if (entity->tags & Level::IS_HOOKABLE &&
                (CheckCollisionPointRec(projectedEnd, entity->hitbox) ||
                CheckCollisionPointRec(projectedEndPartway1, entity->hitbox) ||
                CheckCollisionPointRec(projectedEndPartway2, entity->hitbox) ||
                CheckCollisionPointRec(projectedEndPartway3, entity->hitbox))
            ) {

            // Hook it!

            attachedTo = entity;

            angularVelocity = ANGULAR_VELOCITY_INITIAL;
            if (!isFacingRight) angularVelocity *= -1;

            break;
        }

        entity = (Level::Entity *) entity->next;
    }
}

void GrapplingHook::Swing() {
    
    // I'm not sure why I'm using cos here, the formula uses sin, but that's what worked
    double alpha = -(GRAVITY_ACCEL / currentLength) * cos(currentAngle) - DAMPING_FACTOR * angularVelocity;
    angularVelocity += alpha * SWINGING_TIME_STEP;
    currentAngle += angularVelocity * SWINGING_TIME_STEP;


    // calculate hook's start based on end + new angle
    start.x = end.x + currentLength * cos(currentAngle);
    start.y = end.y + currentLength * sin(currentAngle) * -1; // -1 to compensate for raylib's coord system


    PLAYER_STATE->hookLaunched->UpdatePlayersPos();
}

void GrapplingHook::UpdateStartPos() {

    float x = PLAYER_ENTITY->hitbox.x;
    if (isFacingRight) x += PLAYER_ENTITY->hitbox.width;

    float y = PLAYER_ENTITY->hitbox.y + (PLAYER_ENTITY->hitbox.height/4);
    
    this->start = { x, y };
}

void GrapplingHook::UpdatePlayersPos() {
    
    float x = start.x;
    if (isFacingRight) x -= PLAYER_ENTITY->hitbox.width;

    float y = start.y - (PLAYER_ENTITY->hitbox.height/4);

    PLAYER_ENTITY->hitbox.x = x;
    PLAYER_ENTITY->hitbox.y = y;

    PlayerSyncHitboxes();
}

void GrapplingHook::Draw() {
    
    Vector2 posStart = PosInSceneToScreen(start);
    Vector2 posEnd = PosInSceneToScreen(end);
    DrawLineEx(posStart, posEnd, THICKNESS, RAYWHITE);
}

GrapplingHook::~GrapplingHook() {

    LinkedList::RemoveNode(&Level::STATE->listHead, this);
    PLAYER_STATE->hookLaunched = 0;
    
    TraceLog(LOG_TRACE, "Destroying grappling hook");
}
