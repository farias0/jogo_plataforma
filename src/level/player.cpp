#include <raylib.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "player.hpp"
#include "level.hpp"
#include "enemy.hpp"
#include "grappling_hook.hpp"
#include "../camera.hpp"
#include "../render.hpp"
#include "../sounds.hpp"
#include "../input.hpp"


// What % of the player's height is upperbody, for hitboxes
#define PLAYERS_UPPERBODY_PROPORTION        0.90f

#define X_MAX_SPEED_WALKING                 6.0f
#define X_MAX_SPEED_RUNNING                 9.0f
#define X_ACCELERATION_RATE                 0.25f
#define X_DEACCELERATION_RATE_PASSIVE       0.3f // For when the player just stops moving
#define X_DEACCELERATION_RATE_ACTIVE        0.45f // For when the player actively tries to stop in place

// The Y velocity applied when starting a jump
#define JUMP_START_Y_VELOCITY_DEFAULT                       10.0f
#define JUMP_START_Y_VELOCITY_RUNNING                       12.0f

// The velocities applied when jumping from a hook
#define HOOK_JUMP_Y_VELOCITY_BASE                   04.0f // Base Y speed when jumping from a hook
#define HOOK_JUMP_Y_VELOCITY_FROM_ANGLE             07.5f // How much Y speed can be added to BASE depending on launch angle
#define HOOK_JUMP_Y_VELOCITY_RUNNING_MULTIPLIER     1.40f // How much Y speed is multiplied by if holding 'run'
#define HOOK_JUMP_X_VELOCITY_BASE                   07.5f // Base X boost when jumping from a hook
#define HOOK_JUMP_X_VELOCITY_DIR_MULTIPLIER         1.25f // How much X speed is multiplied by if holding the direction of the jump
#define HOOK_JUMP_X_VELOCITY_RUNNING_MULTIPLIER     1.45f // How much X speed is multiplied by if holding 'run'

#define HOOK_PUSH_ANGULAR_VELOCITY_DELTA            0.0025f // How much the angular velocity of the hook will change
                                                            // when the player is pushing it to one side. In radians/frame.

#define HOOK_ANGULAR_TO_LINEAR_VEL_CONVERSION_RATE  15  // Used when converting hook's angular vel to player's linear vel.
                                                        // Found by testing, changing may lead to bug with collision.

#define DOWNWARDS_VELOCITY_TARGET           -10.0f
#define Y_VELOCITY_TARGET_TOLERANCE         1

#define Y_ACCELERATION_RATE                 0.4f

// A special, constant, Y velocity for when the player is gliding
#define Y_VELOCITY_GLIDING                  -1.5f

// How much of the Y velocity is preserved when the ceiling is hit
// and the trajectory vector is inverted (from upwards to downwards)
#define CEILING_VELOCITY_FACTOR             0.5f


// How many seconds before landing on the ground the jump command
// still works 
#define JUMP_BUFFER_BACKWARDS_SIZE          0.08f  

// JUMP_BUFFER_BACKWARDS_SIZE for when the player is gliding.
// The player is falling slower, and it feels jarring to have  
// a smaller Y window where the backwards jump buffer works.
#define JUMP_BUFFER_BACKWARDS_SIZE_GLIDING  0.20f

// How many seconds after having left the ground the jump command
// still works
#define JUMP_BUFFER_FORWARDS_SIZE           0.15f


Level::Entity *PLAYER_ENTITY = 0;

PlayerState *PLAYER_STATE = 0;


static void initializePlayerState() {

    MemFree(PLAYER_STATE);
    PLAYER_STATE = (PlayerState *) MemAlloc(sizeof(PlayerState));

    PLAYER_STATE->isAscending = false;
    PLAYER_STATE->mode = PLAYER_MODE_DEFAULT;
    PLAYER_STATE->lastPressedJump = -1;
    PLAYER_STATE->lastGroundBeneath = -1;

    TraceLog(LOG_DEBUG, "Player state initialized.");
}

// The vertical velocity that works as the initial
// propulsion of a jump
static float jumpStartVelocity() {

    // Velocity if player's swinging from a hook
    if (PLAYER_STATE->hookLaunched && PLAYER_STATE->hookLaunched->attachedTo) {
        
        auto h = PLAYER_STATE->hookLaunched;
        float vel = HOOK_JUMP_Y_VELOCITY_BASE;
        bool isSwingingClockwise = h->angularVelocity >= 0;
        // where the hook start is in the cartesian plane
        bool isInSecondOrThirdQuadrants = h->currentAngle > PI/2 && h->currentAngle <= (3.0f/2.0f)*PI;

        // adds extra Y velocity if the hook is swinging upwards
        if (isSwingingClockwise && !isInSecondOrThirdQuadrants)
            vel += HOOK_JUMP_Y_VELOCITY_FROM_ANGLE * cos(h->currentAngle);
        else if (!isSwingingClockwise && isInSecondOrThirdQuadrants)
            vel += HOOK_JUMP_Y_VELOCITY_FROM_ANGLE * cos(h->currentAngle) * -1;

        if (Input::STATE.isHoldingRun) vel *= HOOK_JUMP_Y_VELOCITY_RUNNING_MULTIPLIER;

        return vel;
    }

    if (Input::STATE.isHoldingRun)
        return JUMP_START_Y_VELOCITY_RUNNING;

    else
        return JUMP_START_Y_VELOCITY_DEFAULT;
}

// The size of the backwards jump buffer, that varies in function
// of its vertical velocity
static float jumpBufferBackwardsSize() {

    if (PLAYER_STATE->isGliding)
        return JUMP_BUFFER_BACKWARDS_SIZE_GLIDING;
    else
        return JUMP_BUFFER_BACKWARDS_SIZE;
}

static void die() {

    PLAYER_ENTITY->isDead = true;
    Level::STATE->isPaused = true;

    TraceLog(LOG_DEBUG, "You Died.\n\tx=%f, y=%f, isAscending=%d",
                PLAYER_ENTITY->hitbox.x, PLAYER_ENTITY->hitbox.y, PLAYER_STATE->isAscending);
}

static void jump() {

    PLAYER_STATE->isAscending = true;
    PLAYER_STATE->yVelocity = jumpStartVelocity();
    PLAYER_STATE->yVelocityTarget = 0.0f;
    PLAYER_STATE->wasRunningOnJumpStart = Input::STATE.isHoldingRun;
    Sounds::Play(SOUNDS->Jump);
}


void PlayerInitialize(Vector2 origin) {

    Level::Entity *newPlayer = new Level::Entity();
    PLAYER_ENTITY = newPlayer;
    LinkedList::AddNode(&Level::STATE->listHead, newPlayer);
 
    newPlayer->tags = Level::IS_PLAYER;
    newPlayer->origin = origin;
    newPlayer->sprite = SPRITES->PlayerDefault;
    newPlayer->hitbox = SpriteHitboxFromEdge(newPlayer->sprite, newPlayer->origin);
    newPlayer->isFacingRight = true;

    initializePlayerState();
    
    PlayerSyncHitboxes();

    TraceLog(LOG_TRACE, "Added player to level (x=%.1f, y=%.1f)",
                newPlayer->hitbox.x, newPlayer->hitbox.y);
}

void PlayerCheckAndSetOrigin(Vector2 pos) {

    if (!PLAYER_ENTITY) return;

    Rectangle hitbox = SpriteHitboxFromMiddle(SPRITES->PlayerDefault, pos);
    
    if (Level::CheckCollisionWithAnyEntity(hitbox)) {
        TraceLog(LOG_DEBUG,
            "Player's origin couldn't be set at pos x=%.1f, y=%.1f; would collide with a different entity.", pos.x, pos.y);
        Render::PrintSysMessage("Origem iria colidir.");
        return;
    }

    PLAYER_ENTITY->origin = { hitbox.x, hitbox.y };

    TraceLog(LOG_DEBUG, "Player's origin set to x=%.1f, y=%.1f.", PLAYER_ENTITY->origin.x, PLAYER_ENTITY->origin.y);
}

void PlayerCheckAndSetPos(Vector2 pos) {

    if (!PLAYER_ENTITY) return;

    Rectangle hitbox = SpriteHitboxFromMiddle(PLAYER_ENTITY->sprite, pos);
    
    if (Level::CheckCollisionWithAnyEntity(hitbox)) {
        TraceLog(LOG_DEBUG,
            "Player couldn't be set at pos x=%.1f, y=%.1f; would collide with a different entity.", pos.x, pos.y);
        Render::PrintSysMessage("Jogador iria colidir.");
        return;
    }
    
    PLAYER_ENTITY->hitbox = hitbox;
    PlayerSyncHitboxes();
    TraceLog(LOG_DEBUG, "Player set to pos x=%.1f, y=%.1f.", PLAYER_ENTITY->hitbox.x, PLAYER_ENTITY->hitbox.y);
}

void PlayerSyncHitboxes() {

    PLAYER_STATE->upperbody = {
        PLAYER_ENTITY->hitbox.x + 1,
        PLAYER_ENTITY->hitbox.y - 1,
        PLAYER_ENTITY->hitbox.width + 2,
        PLAYER_ENTITY->hitbox.height * PLAYERS_UPPERBODY_PROPORTION + 1
    };

    /*
        playersLowerbody has one extra pixel in its sides and below it.
        This has game feel implications, but it was included so the collision check would work more consistently
        (this seems to be related with the use of raylib's CheckCollisionRecs for player-enemy collision).
        This is something that should not be needed in a more robust implementation.
    */
    PLAYER_STATE->lowerbody = {
        PLAYER_ENTITY->hitbox.x - 1,
        PLAYER_ENTITY->hitbox.y + PLAYER_STATE->upperbody.height,
        PLAYER_ENTITY->hitbox.width + 2,
        PLAYER_ENTITY->hitbox.height * (1 - PLAYERS_UPPERBODY_PROPORTION) + 1
    };
}

void PlayerSetMode(PlayerMode mode) {

    PLAYER_STATE->mode = mode;

    TraceLog(LOG_DEBUG, "Player set mode to %d.", mode);
}

void PlayerJump() {

    if (PLAYER_STATE->hookLaunched && PLAYER_STATE->hookLaunched->attachedTo) {
        
        jump();


        //  Horizontal velocity calculation

        float xVelocity = 0;
        const float angularVel = PLAYER_STATE->hookLaunched->angularVelocity;

        if (PLAYER_ENTITY->isFacingRight && angularVel > 0) { // facing + swinging to the right
            xVelocity = HOOK_JUMP_X_VELOCITY_BASE * sin(PLAYER_STATE->hookLaunched->currentAngle + PI);
        }
        else if (!PLAYER_ENTITY->isFacingRight && angularVel < 0) { // facing + swinging to the left
            xVelocity = HOOK_JUMP_X_VELOCITY_BASE * sin(PLAYER_STATE->hookLaunched->currentAngle + PI) * -1;
        }

        // if the player is holding the direction they're facing
        if ((PLAYER_ENTITY->isFacingRight && Input::STATE.playerMoveDirection == Input::PLAYER_DIRECTION_RIGHT) ||
                (!PLAYER_ENTITY->isFacingRight && Input::STATE.playerMoveDirection == Input::PLAYER_DIRECTION_LEFT))
            xVelocity *= HOOK_JUMP_X_VELOCITY_DIR_MULTIPLIER;

        if (Input::STATE.isHoldingRun)
            xVelocity *= HOOK_JUMP_X_VELOCITY_RUNNING_MULTIPLIER;

        PLAYER_STATE->xVelocity = xVelocity;
        

        delete PLAYER_STATE->hookLaunched;
        
        return;
    }


    // Normal jump from the ground 

    PLAYER_STATE->lastPressedJump = GetTime();
}

void PlayerTick() {

    PlayerState *pState = PLAYER_STATE;
    bool collidedWithTextboxButton = false; // controls the exhibition of textboxes


    if (Level::STATE->concludedAgo >= 0) return;


    bool yVelocityWithinTarget;
    double now;
    const float oldX = PLAYER_ENTITY->hitbox.x;
    const float oldY = PLAYER_ENTITY->hitbox.y;
    const bool isHooked = PLAYER_STATE->hookLaunched && PLAYER_STATE->hookLaunched->attachedTo;


    if (Input::STATE.playerMoveDirection == Input::PLAYER_DIRECTION_RIGHT)
        PLAYER_ENTITY->isFacingRight = true;
    else if (Input::STATE.playerMoveDirection == Input::PLAYER_DIRECTION_LEFT)
        PLAYER_ENTITY->isFacingRight = false;


    if (isHooked) {

        //      Hooked!!
        const auto hook = PLAYER_STATE->hookLaunched;

        // Uses its own system for player's pos calculation
        PLAYER_STATE->xVelocity = 0;
        PLAYER_STATE->yVelocity = 0;
        PLAYER_STATE->yVelocityTarget = 0;


        if (Input::STATE.playerMoveDirection != Input::PLAYER_DIRECTION_STOP &&
                (hook->currentAngle > PI && hook->currentAngle < 2*PI)) {

            // Player push on hook

            const bool isPushingToTheLeft = Input::STATE.playerMoveDirection == Input::PLAYER_DIRECTION_LEFT;
            float angularVelocityDelta = HOOK_PUSH_ANGULAR_VELOCITY_DELTA;
            if (isPushingToTheLeft) angularVelocityDelta *= -1; 
            hook->angularVelocity += angularVelocityDelta;

        }


        hook->Swing();


        // Update player's data according to the hook's new data

        float x = hook->start.x;
        if (hook->isFacingRight) x -= PLAYER_ENTITY->hitbox.width;

        float y = hook->start.y - (PLAYER_ENTITY->hitbox.height/4);

        PLAYER_ENTITY->hitbox.x = x;
        PLAYER_ENTITY->hitbox.y = y;

        PlayerSyncHitboxes();


        PLAYER_STATE->xVelocity = (hook->angularVelocity * HOOK_ANGULAR_TO_LINEAR_VEL_CONVERSION_RATE) * sin(hook->currentAngle) * -1;
        PLAYER_STATE->yVelocity = (hook->angularVelocity * HOOK_ANGULAR_TO_LINEAR_VEL_CONVERSION_RATE) * cos(hook->currentAngle);

        bool isSwingingClockwise = hook->angularVelocity > 0;
        bool isInSecondOrThirdQuadrants = hook->currentAngle > PI/2 && hook->currentAngle <= (3.0f/2.0f)*PI;
        PLAYER_STATE->isAscending = isSwingingClockwise != isInSecondOrThirdQuadrants;


        goto COLISION_CHECKING;
    }


    pState->groundBeneath = Level::GetGroundBeneath(PLAYER_ENTITY);


    { // Horizontal velocity calculation

        float xVelocity = fabs(pState->xVelocity);

        if (Input::STATE.playerMoveDirection == Input::PLAYER_DIRECTION_LEFT && pState->xVelocity > 0) {

            pState->xVelocity -= X_DEACCELERATION_RATE_ACTIVE;
            if (pState->xVelocity < 0) pState->xVelocity = 0;
            goto END_HORIZONTAL_VELOCITY_CALCULATION;
        }
        else if (Input::STATE.playerMoveDirection == Input::PLAYER_DIRECTION_RIGHT && pState->xVelocity < 0) {

            pState->xVelocity += X_DEACCELERATION_RATE_ACTIVE;
            if (pState->xVelocity > 0) pState->xVelocity = 0;
            goto END_HORIZONTAL_VELOCITY_CALCULATION;
        }

        if (Input::STATE.playerMoveDirection == Input::PLAYER_DIRECTION_STOP) {
            xVelocity -= X_DEACCELERATION_RATE_PASSIVE;
            if (xVelocity < 0) xVelocity = 0;
        }

        else if (Input::STATE.isHoldingRun & (pState->groundBeneath || pState->wasRunningOnJumpStart)) {
            if (xVelocity < X_MAX_SPEED_RUNNING) xVelocity += X_ACCELERATION_RATE;
            else xVelocity -= X_DEACCELERATION_RATE_PASSIVE;
        }
        
        else {
            if (xVelocity < X_MAX_SPEED_WALKING) xVelocity += X_ACCELERATION_RATE;
            else xVelocity -= X_DEACCELERATION_RATE_PASSIVE;
        }

        if (!PLAYER_ENTITY->isFacingRight) xVelocity *= -1;

        pState->xVelocity = xVelocity;
    }
END_HORIZONTAL_VELOCITY_CALCULATION:


    if (pState->groundBeneath) {

        PLAYER_STATE->lastGroundBeneath = GetTime();

        if (!pState->isAscending) {
            // Is on the ground
            PLAYER_ENTITY->hitbox.y = pState->groundBeneath->hitbox.y - PLAYER_ENTITY->hitbox.height;
            pState->yVelocity = 0;
            pState->yVelocityTarget = 0;
        }
    }


    yVelocityWithinTarget =
        abs((int) (pState->yVelocity - pState->yVelocityTarget)) < Y_VELOCITY_TARGET_TOLERANCE;
    
    if (yVelocityWithinTarget) {
        pState->isAscending = false;

        if (!pState->groundBeneath) {
            // Starts falling down
            pState->yVelocityTarget = DOWNWARDS_VELOCITY_TARGET;
        }
    }

    now = GetTime();
    if (!pState->isAscending &&
        (now - PLAYER_STATE->lastPressedJump < jumpBufferBackwardsSize()) &&
        (now - PLAYER_STATE->lastGroundBeneath < JUMP_BUFFER_FORWARDS_SIZE)) {

        jump();

        // Player hit enemy
        // -- this check is for the case the player jumps off the enemy,
        // and only in the case the enemy wasn't already destroyed in the previous frame.
        if (pState->groundBeneath &&
            pState->groundBeneath->tags & Level::IS_ENEMY) {

            PLAYER_STATE->lastGroundBeneath = GetTime();
            EnemyKill(pState->groundBeneath);
            pState->groundBeneath = 0;
        }
    }

    PLAYER_ENTITY->hitbox.x += pState->xVelocity;
    PLAYER_ENTITY->hitbox.y -= pState->yVelocity;
    PlayerSyncHitboxes();


COLISION_CHECKING:
    // Collision checking
    {
        if (PLAYER_ENTITY->hitbox.y + PLAYER_ENTITY->hitbox.height > FLOOR_DEATH_HEIGHT) {
            die();
            return;
        }

        Level::Entity *entity = (Level::Entity *) Level::STATE->listHead;

        while (entity != 0) {

            if ((entity->tags & Level::IS_ENEMY) && !entity->isDead) {

                // Enemy hit player
                if (CheckCollisionRecs(entity->hitbox, pState->upperbody)) {
                    die();
                    break;
                }

                // Player hit enemy
                if (CheckCollisionRecs(entity->hitbox, pState->lowerbody)) {
                    PLAYER_STATE->lastGroundBeneath = GetTime();
                    EnemyKill(entity);
                    goto next_entity;
                }
            }

            else if (entity->tags & Level::IS_SCENARIO) {

                // Check for collision with level geometry

                Rectangle collisionRec = GetCollisionRec(entity->hitbox, PLAYER_ENTITY->hitbox);

                if (entity->tags & Level::IS_DANGER &&
                    (collisionRec.width > 0 || collisionRec.height > 0 || pState->groundBeneath == entity)) {
                    
                    // Player hit dangerous level element
                    die();
                    break;
                }

                if (collisionRec.width == 0 || collisionRec.height == 0) goto next_entity;
            
                const bool isAWall = collisionRec.width <= collisionRec.height;
                const bool isACeiling = (collisionRec.width >= collisionRec.height) &&
                                    (entity->hitbox.y < PLAYER_ENTITY->hitbox.y);


                if (isAWall && collisionRec.width > 0) {

                    const bool isEntitysRightWall = collisionRec.x > entity->hitbox.x; 
                    const bool isEntitysLeftWall = !isEntitysRightWall;

                    const bool isEntityToTheLeft = entity->hitbox.x < PLAYER_ENTITY->hitbox.x;
                    const bool isEntityToTheRight = !isEntityToTheLeft;

                    const bool isPlayerMovingToTheLeft = PLAYER_ENTITY->hitbox.x < oldX;
                    const bool isPlayerMovingToTheRight = PLAYER_ENTITY->hitbox.x > oldX;

                    // So when the player hits the entity to its left,
                    // he can only collide with its left wall.
                    if ((isEntitysRightWall && isEntityToTheLeft && isPlayerMovingToTheLeft) ||
                        (isEntitysLeftWall && isEntityToTheRight && isPlayerMovingToTheRight)) {


                        // if (GAME_STATE->showDebugHUD) Render::PrintSysMessage("Hit wall");

                        PLAYER_ENTITY->hitbox.x = oldX;

                        if (isHooked) pState->hookLaunched->angularVelocity *= -1;

                    }
                }


                if (isACeiling && pState->isAscending) {

                    // if (GAME_STATE->showDebugHUD) Render::PrintSysMessage("Hit ceiling");
                    
                    pState->isAscending = false;
                    PLAYER_ENTITY->hitbox.y = oldY;
                    pState->yVelocity = (pState->yVelocity * -1) * CEILING_VELOCITY_FACTOR;
                    pState->yVelocityTarget = DOWNWARDS_VELOCITY_TARGET;

                    if (isHooked) pState->hookLaunched->angularVelocity *= -1;
                }

                // TODO maybe ground check should be here as well
            }

            else if (entity->tags & Level::IS_EXIT &&
                        CheckCollisionRecs(entity->hitbox, PLAYER_ENTITY->hitbox)) {

                // Player exit level

                Level::GoToOverworld();
            }

            else if (entity->tags & Level::IS_GLIDE &&
                        CheckCollisionRecs(entity->hitbox, PLAYER_ENTITY->hitbox) &&
                        PLAYER_STATE->mode != PLAYER_MODE_GLIDE) {
                PlayerSetMode(PLAYER_MODE_GLIDE);
                return;
            }

            else if (entity->tags & Level::IS_TEXTBOX &&
                        CheckCollisionRecs(entity->hitbox, PLAYER_ENTITY->hitbox)) {

                collidedWithTextboxButton = true;

                if (Render::GetTextboxTextId() == -1) {
                    Render::DisplayTextbox(entity->textId);
                    TraceLog(LOG_TRACE, "Textbox started displaying textId=%d.", entity->textId);
                }
            }

next_entity:
            entity = (Level::Entity *)  entity->next;
        }
    }


    PLAYER_STATE->isGliding = false;
    if (pState->yVelocity > pState->yVelocityTarget && !isHooked) {

        if (PLAYER_STATE->mode == PLAYER_MODE_GLIDE &&
                !PLAYER_STATE->isAscending &&
                Input::STATE.isHoldingRun) {

            // Is gliding
            pState->yVelocity = Y_VELOCITY_GLIDING;
            PLAYER_STATE->isGliding = true;

        } else {

            // Accelerates jump's vertical movement
            pState->yVelocity -= Y_ACCELERATION_RATE;
        }
    }

    if (isHooked) PLAYER_STATE->hookLaunched->FollowPlayer();

    if (Render::GetTextboxTextId() != -1 && !collidedWithTextboxButton) {
        Render::DisplayTextboxStop();
        TraceLog(LOG_TRACE, "Textbox stopped displaying.");
    }


    // "Animation"
    Sprite currentSprite;

    if (PLAYER_STATE->mode == PLAYER_MODE_GLIDE) {
        if (PLAYER_STATE->isGliding)
            currentSprite =     SPRITES->PlayerGlideFalling;
        else
            currentSprite =     SPRITES->PlayerGlideOn;
    }
    else currentSprite =        SPRITES->PlayerDefault;

    PLAYER_ENTITY->sprite.sprite = currentSprite.sprite;
}

void PlayerContinue() {

    Level::STATE->isPaused = false;

    // Reset all the entities to their origins
    Level::Entity *entity = (Level::Entity *) Level::STATE->listHead;
    while (entity) {

        entity->isDead = false;
        entity->hitbox.x = entity->origin.x;
        entity->hitbox.y = entity->origin.y;

        entity = (Level::Entity *) entity->next;
    }

    if (Level::STATE->checkpoint) {
        Vector2 pos = RectangleGetPos(Level::STATE->checkpoint->hitbox);
        pos.y -= Level::STATE->checkpoint->hitbox.height;
        RectangleSetPos(&PLAYER_ENTITY->hitbox, pos);
    } else {
        RectangleSetPos(&PLAYER_ENTITY->hitbox, PLAYER_ENTITY->origin);
    }
    PLAYER_STATE->isAscending = false;
    PLAYER_STATE->isGliding = false;
    PLAYER_STATE->yVelocity = 0;
    PLAYER_STATE->yVelocityTarget = 0;
    PLAYER_STATE->xVelocity = 0;

    if (PLAYER_STATE->hookLaunched) delete PLAYER_STATE->hookLaunched;

    PlayerSyncHitboxes();
    CameraLevelCentralizeOnPlayer();

    TraceLog(LOG_DEBUG, "Player continue.");
}

void PlayerSetCheckpoint() {

    if (!PLAYER_STATE->groundBeneath) {
        TraceLog(LOG_DEBUG, "Player didn't set checkpoint, not on the ground.");
        return;
    }

    if (Level::STATE->checkpointsLeft < 1) {
        Render::PrintSysMessage("Sem checkpoints disponíveis.");
        return;
    }

    if (Level::STATE->checkpoint) {
        Level::EntityDestroy(Level::STATE->checkpoint);
    }

    Vector2 pos = RectangleGetPos(PLAYER_ENTITY->hitbox);
    pos.y += PLAYER_ENTITY->hitbox.height / 2;
    Level::STATE->checkpoint = Level::CheckpointAdd(pos);

    Level::STATE->checkpointsLeft--;

    TraceLog(LOG_DEBUG, "Player set checkpoint at x=%.1f, y=%.1f.", pos.x, pos.y);
}

void PlayerLaunchGrapplingHook() {

    if (PLAYER_STATE->hookLaunched) {
        delete PLAYER_STATE->hookLaunched;
        return;
    }

    PLAYER_STATE->hookLaunched = GrapplingHook::Initialize();
}
