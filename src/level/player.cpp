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


Player *PLAYER = 0;


// The vertical velocity that works as the initial
// propulsion of a jump
static float jumpStartVelocity() {

    // Velocity if player's swinging from a hook
    if (PLAYER->hookLaunched && PLAYER->hookLaunched->attachedTo) {
        
        auto h = PLAYER->hookLaunched;
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

    if (PLAYER->isGliding)
        return JUMP_BUFFER_BACKWARDS_SIZE_GLIDING;
    else
        return JUMP_BUFFER_BACKWARDS_SIZE;
}

static void die() {

    PLAYER->isDead = true;
    Level::STATE->isPaused = true;

    TraceLog(LOG_DEBUG, "You Died.\n\tx=%f, y=%f, isAscending=%d",
                PLAYER->hitbox.x, PLAYER->hitbox.y, PLAYER->isAscending);
}

static void jump() {

    PLAYER->isAscending = true;
    PLAYER->yVelocity = jumpStartVelocity();
    PLAYER->yVelocityTarget = 0.0f;
    PLAYER->wasRunningOnJumpStart = Input::STATE.isHoldingRun;
    Sounds::Play(SOUNDS->Jump);
}


void Player::Initialize(Vector2 origin) {

    Player *newPlayer = new Player();
    PLAYER = newPlayer;
    LinkedList::AddNode(&Level::STATE->listHead, newPlayer);
 
    newPlayer->tags = Level::IS_PLAYER;
    newPlayer->origin = origin;
    newPlayer->sprite = SPRITES->PlayerDefault;
    newPlayer->hitbox = SpriteHitboxFromEdge(newPlayer->sprite, newPlayer->origin);
    newPlayer->isFacingRight = true;

    newPlayer->isAscending = false;
    newPlayer->mode = PLAYER_MODE_DEFAULT;
    newPlayer->lastPressedJump = -1;
    newPlayer->lastGroundBeneath = -1;
    
    newPlayer->SyncHitboxes();

    TraceLog(LOG_TRACE, "Added player to level (x=%.1f, y=%.1f)",
                newPlayer->hitbox.x, newPlayer->hitbox.y);
}

void Player::CheckAndSetOrigin(Vector2 pos) {

    if (!PLAYER) return;

    Rectangle hitbox = SpriteHitboxFromMiddle(SPRITES->PlayerDefault, pos);
    
    if (Level::CheckCollisionWithAnyEntity(hitbox)) {
        TraceLog(LOG_DEBUG,
            "Player's origin couldn't be set at pos x=%.1f, y=%.1f; would collide with a different entity.", pos.x, pos.y);
        Render::PrintSysMessage("Origem iria colidir.");
        return;
    }

    PLAYER->origin = { hitbox.x, hitbox.y };

    TraceLog(LOG_DEBUG, "Player's origin set to x=%.1f, y=%.1f.", PLAYER->origin.x, PLAYER->origin.y);
}

void Player::CheckAndSetPos(Vector2 pos) {

    if (!PLAYER) return;

    Rectangle hitbox = SpriteHitboxFromMiddle(PLAYER->sprite, pos);
    
    if (Level::CheckCollisionWithAnyEntity(hitbox)) {
        TraceLog(LOG_DEBUG,
            "Player couldn't be set at pos x=%.1f, y=%.1f; would collide with a different entity.", pos.x, pos.y);
        Render::PrintSysMessage("Jogador iria colidir.");
        return;
    }
    
    PLAYER->hitbox = hitbox;
    PLAYER->SyncHitboxes();
    TraceLog(LOG_DEBUG, "Player set to pos x=%.1f, y=%.1f.", PLAYER->hitbox.x, PLAYER->hitbox.y);
}

void Player::SyncHitboxes() {

    PLAYER->upperbody = {
        PLAYER->hitbox.x + 1,
        PLAYER->hitbox.y - 1,
        PLAYER->hitbox.width + 2,
        PLAYER->hitbox.height * PLAYERS_UPPERBODY_PROPORTION + 1
    };

    /*
        playersLowerbody has one extra pixel in its sides and below it.
        This has game feel implications, but it was included so the collision check would work more consistently
        (this seems to be related with the use of raylib's CheckCollisionRecs for player-enemy collision).
        This is something that should not be needed in a more robust implementation.
    */
    PLAYER->lowerbody = {
        PLAYER->hitbox.x - 1,
        PLAYER->hitbox.y + PLAYER->upperbody.height,
        PLAYER->hitbox.width + 2,
        PLAYER->hitbox.height * (1 - PLAYERS_UPPERBODY_PROPORTION) + 1
    };
}

void Player::SetMode(PlayerMode mode) {

    PLAYER->mode = mode;

    TraceLog(LOG_DEBUG, "Player set mode to %d.", mode);
}

void Player::Jump() {

    if (PLAYER->hookLaunched && PLAYER->hookLaunched->attachedTo) {
        
        jump();


        //  Horizontal velocity calculation

        float xVelocity = 0;
        const float angularVel = PLAYER->hookLaunched->angularVelocity;

        if (PLAYER->isFacingRight && angularVel > 0) { // facing + swinging to the right
            xVelocity = HOOK_JUMP_X_VELOCITY_BASE * sin(PLAYER->hookLaunched->currentAngle + PI);
        }
        else if (!PLAYER->isFacingRight && angularVel < 0) { // facing + swinging to the left
            xVelocity = HOOK_JUMP_X_VELOCITY_BASE * sin(PLAYER->hookLaunched->currentAngle + PI) * -1;
        }

        // if the player is holding the direction they're facing
        if ((PLAYER->isFacingRight && Input::STATE.playerMoveDirection == Input::PLAYER_DIRECTION_RIGHT) ||
                (!PLAYER->isFacingRight && Input::STATE.playerMoveDirection == Input::PLAYER_DIRECTION_LEFT))
            xVelocity *= HOOK_JUMP_X_VELOCITY_DIR_MULTIPLIER;

        if (Input::STATE.isHoldingRun)
            xVelocity *= HOOK_JUMP_X_VELOCITY_RUNNING_MULTIPLIER;

        PLAYER->xVelocity = xVelocity;
        

        delete PLAYER->hookLaunched;
        
        return;
    }


    // Normal jump from the ground 

    PLAYER->lastPressedJump = GetTime();
}

void Player::Tick() {

    bool collidedWithTextboxButton = false; // controls the exhibition of textboxes


    if (Level::STATE->concludedAgo >= 0) return;


    bool yVelocityWithinTarget;
    double now;
    const float oldX = PLAYER->hitbox.x;
    const float oldY = PLAYER->hitbox.y;
    const bool isHooked = PLAYER->hookLaunched && PLAYER->hookLaunched->attachedTo;


    if (Input::STATE.playerMoveDirection == Input::PLAYER_DIRECTION_RIGHT)
        PLAYER->isFacingRight = true;
    else if (Input::STATE.playerMoveDirection == Input::PLAYER_DIRECTION_LEFT)
        PLAYER->isFacingRight = false;


    if (isHooked) {

        //      Hooked!!
        const auto hook = PLAYER->hookLaunched;

        // Uses its own system for player's pos calculation
        PLAYER->xVelocity = 0;
        PLAYER->yVelocity = 0;
        PLAYER->yVelocityTarget = 0;


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
        if (hook->isFacingRight) x -= PLAYER->hitbox.width;

        float y = hook->start.y - (PLAYER->hitbox.height/4);

        PLAYER->hitbox.x = x;
        PLAYER->hitbox.y = y;

        PLAYER->SyncHitboxes();


        PLAYER->xVelocity = (hook->angularVelocity * HOOK_ANGULAR_TO_LINEAR_VEL_CONVERSION_RATE) * sin(hook->currentAngle) * -1;
        PLAYER->yVelocity = (hook->angularVelocity * HOOK_ANGULAR_TO_LINEAR_VEL_CONVERSION_RATE) * cos(hook->currentAngle);

        bool isSwingingClockwise = hook->angularVelocity > 0;
        bool isInSecondOrThirdQuadrants = hook->currentAngle > PI/2 && hook->currentAngle <= (3.0f/2.0f)*PI;
        PLAYER->isAscending = isSwingingClockwise != isInSecondOrThirdQuadrants;


        goto COLISION_CHECKING;
    }


    PLAYER->groundBeneath = Level::GetGroundBeneath(PLAYER);


    { // Horizontal velocity calculation

        float xVelocity = fabs(PLAYER->xVelocity);

        if (Input::STATE.playerMoveDirection == Input::PLAYER_DIRECTION_LEFT && PLAYER->xVelocity > 0) {

            PLAYER->xVelocity -= X_DEACCELERATION_RATE_ACTIVE;
            if (PLAYER->xVelocity < 0) PLAYER->xVelocity = 0;
            goto END_HORIZONTAL_VELOCITY_CALCULATION;
        }
        else if (Input::STATE.playerMoveDirection == Input::PLAYER_DIRECTION_RIGHT && PLAYER->xVelocity < 0) {

            PLAYER->xVelocity += X_DEACCELERATION_RATE_ACTIVE;
            if (PLAYER->xVelocity > 0) PLAYER->xVelocity = 0;
            goto END_HORIZONTAL_VELOCITY_CALCULATION;
        }

        if (Input::STATE.playerMoveDirection == Input::PLAYER_DIRECTION_STOP) {
            xVelocity -= X_DEACCELERATION_RATE_PASSIVE;
            if (xVelocity < 0) xVelocity = 0;
        }

        else if (Input::STATE.isHoldingRun & (PLAYER->groundBeneath || PLAYER->wasRunningOnJumpStart)) {
            if (xVelocity < X_MAX_SPEED_RUNNING) xVelocity += X_ACCELERATION_RATE;
            else xVelocity -= X_DEACCELERATION_RATE_PASSIVE;
        }
        
        else {
            if (xVelocity < X_MAX_SPEED_WALKING) xVelocity += X_ACCELERATION_RATE;
            else xVelocity -= X_DEACCELERATION_RATE_PASSIVE;
        }

        if (!PLAYER->isFacingRight) xVelocity *= -1;

        PLAYER->xVelocity = xVelocity;
    }
END_HORIZONTAL_VELOCITY_CALCULATION:


    if (PLAYER->groundBeneath) {

        PLAYER->lastGroundBeneath = GetTime();

        if (!PLAYER->isAscending) {
            // Is on the ground
            PLAYER->hitbox.y = PLAYER->groundBeneath->hitbox.y - PLAYER->hitbox.height;
            PLAYER->yVelocity = 0;
            PLAYER->yVelocityTarget = 0;
        }
    }


    yVelocityWithinTarget =
        abs((int) (PLAYER->yVelocity - PLAYER->yVelocityTarget)) < Y_VELOCITY_TARGET_TOLERANCE;
    
    if (yVelocityWithinTarget) {
        PLAYER->isAscending = false;

        if (!PLAYER->groundBeneath) {
            // Starts falling down
            PLAYER->yVelocityTarget = DOWNWARDS_VELOCITY_TARGET;
        }
    }

    now = GetTime();
    if (!PLAYER->isAscending &&
        (now - PLAYER->lastPressedJump < jumpBufferBackwardsSize()) &&
        (now - PLAYER->lastGroundBeneath < JUMP_BUFFER_FORWARDS_SIZE)) {

        jump();

        // Player hit enemy
        // -- this check is for the case the player jumps off the enemy,
        // and only in the case the enemy wasn't already destroyed in the previous frame.
        if (PLAYER->groundBeneath &&
            PLAYER->groundBeneath->tags & Level::IS_ENEMY) {

            PLAYER->lastGroundBeneath = GetTime();
            EnemyKill(PLAYER->groundBeneath);
            PLAYER->groundBeneath = 0;
        }
    }

    PLAYER->hitbox.x += PLAYER->xVelocity;
    PLAYER->hitbox.y -= PLAYER->yVelocity;
    PLAYER->SyncHitboxes();


COLISION_CHECKING:
    // Collision checking
    {
        if (PLAYER->hitbox.y + PLAYER->hitbox.height > FLOOR_DEATH_HEIGHT) {
            die();
            return;
        }

        Level::Entity *entity = (Level::Entity *) Level::STATE->listHead;

        while (entity != 0) {

            if ((entity->tags & Level::IS_ENEMY) && !entity->isDead) {

                // Enemy hit player
                if (CheckCollisionRecs(entity->hitbox, PLAYER->upperbody)) {
                    die();
                    break;
                }

                // Player hit enemy
                if (CheckCollisionRecs(entity->hitbox, PLAYER->lowerbody)) {
                    PLAYER->lastGroundBeneath = GetTime();
                    EnemyKill(entity);
                    goto next_entity;
                }
            }

            else if (entity->tags & Level::IS_SCENARIO) {

                // Check for collision with level geometry

                Rectangle collisionRec = GetCollisionRec(entity->hitbox, PLAYER->hitbox);

                if (entity->tags & Level::IS_DANGER &&
                    (collisionRec.width > 0 || collisionRec.height > 0 || PLAYER->groundBeneath == entity)) {
                    
                    // Player hit dangerous level element
                    die();
                    break;
                }

                if (collisionRec.width == 0 || collisionRec.height == 0) goto next_entity;
            
                const bool isAWall = collisionRec.width <= collisionRec.height;
                const bool isACeiling = (collisionRec.width >= collisionRec.height) &&
                                    (entity->hitbox.y < PLAYER->hitbox.y);


                if (isAWall && collisionRec.width > 0) {

                    const bool isEntitysRightWall = collisionRec.x > entity->hitbox.x; 
                    const bool isEntitysLeftWall = !isEntitysRightWall;

                    const bool isEntityToTheLeft = entity->hitbox.x < PLAYER->hitbox.x;
                    const bool isEntityToTheRight = !isEntityToTheLeft;

                    const bool isPlayerMovingToTheLeft = PLAYER->hitbox.x < oldX;
                    const bool isPlayerMovingToTheRight = PLAYER->hitbox.x > oldX;

                    // So when the player hits the entity to its left,
                    // he can only collide with its left wall.
                    if ((isEntitysRightWall && isEntityToTheLeft && isPlayerMovingToTheLeft) ||
                        (isEntitysLeftWall && isEntityToTheRight && isPlayerMovingToTheRight)) {


                        // if (GAME_STATE->showDebugHUD) Render::PrintSysMessage("Hit wall");

                        PLAYER->hitbox.x = oldX;

                        if (isHooked) PLAYER->hookLaunched->angularVelocity *= -1;

                    }
                }


                if (isACeiling && PLAYER->isAscending) {

                    // if (GAME_STATE->showDebugHUD) Render::PrintSysMessage("Hit ceiling");
                    
                    PLAYER->isAscending = false;
                    PLAYER->hitbox.y = oldY;
                    PLAYER->yVelocity = (PLAYER->yVelocity * -1) * CEILING_VELOCITY_FACTOR;
                    PLAYER->yVelocityTarget = DOWNWARDS_VELOCITY_TARGET;

                    if (isHooked) PLAYER->hookLaunched->angularVelocity *= -1;
                }

                // TODO maybe ground check should be here as well
            }

            else if (entity->tags & Level::IS_EXIT &&
                        CheckCollisionRecs(entity->hitbox, PLAYER->hitbox)) {

                // Player exit level

                Level::GoToOverworld();
            }

            else if (entity->tags & Level::IS_GLIDE &&
                        CheckCollisionRecs(entity->hitbox, PLAYER->hitbox) &&
                        PLAYER->mode != PLAYER_MODE_GLIDE) {
                PLAYER->SetMode(PLAYER_MODE_GLIDE);
                return;
            }

            else if (entity->tags & Level::IS_TEXTBOX &&
                        CheckCollisionRecs(entity->hitbox, PLAYER->hitbox)) {

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


    PLAYER->isGliding = false;
    if (PLAYER->yVelocity > PLAYER->yVelocityTarget && !isHooked) {

        if (PLAYER->mode == PLAYER_MODE_GLIDE &&
                !PLAYER->isAscending &&
                Input::STATE.isHoldingRun) {

            // Is gliding
            PLAYER->yVelocity = Y_VELOCITY_GLIDING;
            PLAYER->isGliding = true;

        } else {

            // Accelerates jump's vertical movement
            PLAYER->yVelocity -= Y_ACCELERATION_RATE;
        }
    }

    if (isHooked) PLAYER->hookLaunched->FollowPlayer();

    if (Render::GetTextboxTextId() != -1 && !collidedWithTextboxButton) {
        Render::DisplayTextboxStop();
        TraceLog(LOG_TRACE, "Textbox stopped displaying.");
    }


    // "Animation"
    Sprite currentSprite;

    if (PLAYER->mode == PLAYER_MODE_GLIDE) {
        if (PLAYER->isGliding)
            currentSprite =     SPRITES->PlayerGlideFalling;
        else
            currentSprite =     SPRITES->PlayerGlideOn;
    }

    // else if (PLAYER->groundBeneath && abs(PLAYER->xVelocity) > 0.5)
    //     currentSprite= SPRITES->PlayerWalking1;

    else currentSprite =        SPRITES->PlayerDefault;

    PLAYER->sprite.sprite = currentSprite.sprite;
}

void Player::Continue() {

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
        RectangleSetPos(&PLAYER->hitbox, pos);
    } else {
        RectangleSetPos(&PLAYER->hitbox, PLAYER->origin);
    }
    PLAYER->isAscending = false;
    PLAYER->isGliding = false;
    PLAYER->yVelocity = 0;
    PLAYER->yVelocityTarget = 0;
    PLAYER->xVelocity = 0;

    if (PLAYER->hookLaunched) delete PLAYER->hookLaunched;

    PLAYER->SyncHitboxes();
    CameraLevelCentralizeOnPlayer();

    TraceLog(LOG_DEBUG, "Player continue.");
}

void Player::SetCheckpoint() {

    if (!PLAYER->groundBeneath) {
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

    Vector2 pos = RectangleGetPos(PLAYER->hitbox);
    pos.y += PLAYER->hitbox.height / 2;
    Level::STATE->checkpoint = Level::CheckpointAdd(pos);

    Level::STATE->checkpointsLeft--;

    TraceLog(LOG_DEBUG, "Player set checkpoint at x=%.1f, y=%.1f.", pos.x, pos.y);
}

void Player::LaunchGrapplingHook() {

    if (PLAYER->hookLaunched) {
        delete PLAYER->hookLaunched;
        return;
    }

    PLAYER->hookLaunched = GrapplingHook::Initialize();
}
