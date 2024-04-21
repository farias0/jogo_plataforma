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


void Player::Initialize(Vector2 origin) {

    Player *newPlayer = new Player();
    PLAYER = newPlayer;
    LinkedList::AddNode(&Level::STATE->listHead, newPlayer);
 
    newPlayer->tags = Level::IS_PLAYER;
    newPlayer->origin = origin;
    newPlayer->sprite = &SPRITES->PlayerDefault;
    newPlayer->SetHitbox(SpriteHitboxFromEdge(newPlayer->sprite, newPlayer->origin));
    newPlayer->isFacingRight = true;

    newPlayer->isAscending = false;
    newPlayer->mode = PLAYER_MODE_DEFAULT;
    newPlayer->lastPressedJump = -1;
    newPlayer->lastGroundBeneath = -1;


    newPlayer->createAnimations();


    TraceLog(LOG_TRACE, "Added player to level (x=%.1f, y=%.1f)",
                newPlayer->hitbox.x, newPlayer->hitbox.y);
}

void Player::CheckAndSetOrigin(Vector2 pos) {

    if (!PLAYER) return;

    Rectangle newHitbox = SpriteHitboxFromMiddle(&SPRITES->PlayerDefault, pos);
    
    if (Level::CheckCollisionWithAnyEntity(newHitbox)) {
        TraceLog(LOG_DEBUG,
            "Player's origin couldn't be set at pos x=%.1f, y=%.1f; would collide with a different entity.", pos.x, pos.y);
        Render::PrintSysMessage("Origem iria colidir.");
        return;
    }

    origin = { newHitbox.x, newHitbox.y };

    TraceLog(LOG_DEBUG, "Player's origin set to x=%.1f, y=%.1f.", origin.x, origin.y);
}

void Player::CheckAndSetPos(Vector2 pos) {

    if (!PLAYER) return;

    Rectangle newHitbox = SpriteHitboxFromMiddle(sprite, pos);
    
    if (Level::CheckCollisionWithAnyEntity(newHitbox)) {
        TraceLog(LOG_DEBUG,
            "Player couldn't be set at pos x=%.1f, y=%.1f; would collide with a different entity.", pos.x, pos.y);
        Render::PrintSysMessage("Jogador iria colidir.");
        return;
    }
    
    SetHitbox(newHitbox);

    TraceLog(LOG_DEBUG, "Player set to pos x=%.1f, y=%.1f.", newHitbox.x, newHitbox.y);
}

void Player::SetHitbox(Rectangle newHitbox) {

    SetPos({ newHitbox.x, newHitbox.y });
    this->hitbox = newHitbox;

}

void Player::SetPos(Vector2 pos) {

    hitbox.x = pos.x;
    hitbox.y = pos.y;

    upperbody = {
        hitbox.x + 1,
        hitbox.y - 1,
        hitbox.width + 2,
        hitbox.height * PLAYERS_UPPERBODY_PROPORTION + 1
    };

    /*
        playersLowerbody has one extra pixel in its sides and below it.
        This has game feel implications, but it was included so the collision check would work more consistently
        (this seems to be related with the use of raylib's CheckCollisionRecs for player-enemy collision).
        This is something that should not be needed in a more robust implementation.
    */
    lowerbody = {
        hitbox.x - 1,
        hitbox.y + upperbody.height,
        hitbox.width + 2,
        hitbox.height * (1 - PLAYERS_UPPERBODY_PROPORTION) + 1
    };
}

void Player::SetMode(PlayerMode newMode) {

    mode = newMode;

    TraceLog(LOG_DEBUG, "Player set mode to %d.", newMode);
}

void Player::Jump() {

    if (hookLaunched && hookLaunched->attachedTo) {
        
        jump();


        //  Horizontal velocity calculation

        float newXVel = 0;
        const float angularVel = hookLaunched->angularVelocity;

        if (isFacingRight && angularVel > 0) { // facing + swinging to the right
            newXVel = HOOK_JUMP_X_VELOCITY_BASE * sin(hookLaunched->currentAngle + PI);
        }
        else if (!isFacingRight && angularVel < 0) { // facing + swinging to the left
            newXVel = HOOK_JUMP_X_VELOCITY_BASE * sin(hookLaunched->currentAngle + PI) * -1;
        }

        // if the player is holding the direction they're facing
        if ((isFacingRight && Input::STATE.playerMoveDirection == Input::PLAYER_DIRECTION_RIGHT) ||
                (!isFacingRight && Input::STATE.playerMoveDirection == Input::PLAYER_DIRECTION_LEFT))
            newXVel *= HOOK_JUMP_X_VELOCITY_DIR_MULTIPLIER;

        if (Input::STATE.isHoldingRun)
            newXVel *= HOOK_JUMP_X_VELOCITY_RUNNING_MULTIPLIER;

        xVelocity = newXVel;
        

        delete hookLaunched;
        
        return;
    }


    // Normal jump from the ground 

    lastPressedJump = GetTime();
}

void Player::Tick() {

    bool collidedWithTextboxButton = false; // controls the exhibition of textboxes


    if (Level::STATE->concludedAgo >= 0) return;


    bool yVelocityWithinTarget;
    double now;
    const float oldX = hitbox.x;
    const float oldY = hitbox.y;
    const bool isHooked = hookLaunched && hookLaunched->attachedTo;


    if (isHooked) {

        //      Hooked!!
        const auto hook = hookLaunched;

        // Uses its own system for player's pos calculation
        xVelocity = 0;
        yVelocity = 0;
        yVelocityTarget = 0;


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
        if (hook->isFacingRight) x -= hitbox.width;

        float y = hook->start.y - (hitbox.height/4);

        SetPos({ x, y });


        xVelocity = (hook->angularVelocity * HOOK_ANGULAR_TO_LINEAR_VEL_CONVERSION_RATE) * sin(hook->currentAngle) * -1;
        yVelocity = (hook->angularVelocity * HOOK_ANGULAR_TO_LINEAR_VEL_CONVERSION_RATE) * cos(hook->currentAngle);

        bool isSwingingClockwise = hook->angularVelocity > 0;
        bool isInSecondOrThirdQuadrants = hook->currentAngle > PI/2 && hook->currentAngle <= (3.0f/2.0f)*PI;
        isAscending = isSwingingClockwise != isInSecondOrThirdQuadrants;


        goto COLISION_CHECKING;
    }


    if (Input::STATE.playerMoveDirection == Input::PLAYER_DIRECTION_RIGHT)
        isFacingRight = true;
    else if (Input::STATE.playerMoveDirection == Input::PLAYER_DIRECTION_LEFT)
        isFacingRight = false;


    groundBeneath = Level::GetGroundBeneath(PLAYER);


    { // Horizontal velocity calculation

        float newXVel = fabs(xVelocity);

        if (Input::STATE.playerMoveDirection == Input::PLAYER_DIRECTION_LEFT && xVelocity > 0) {

            xVelocity -= X_DEACCELERATION_RATE_ACTIVE;
            if (xVelocity < 0) xVelocity = 0;
            if (xVelocity > X_MAX_SPEED_WALKING) isSkidding = true; // becomes true after a threshold,
                                                                    // and only becomes false again when it stops skidding
            goto END_HORIZONTAL_VELOCITY_CALCULATION;
        }
        else if (Input::STATE.playerMoveDirection == Input::PLAYER_DIRECTION_RIGHT && xVelocity < 0) {

            xVelocity += X_DEACCELERATION_RATE_ACTIVE;
            if (xVelocity > 0) xVelocity = 0;
            if (xVelocity < -1 * X_MAX_SPEED_WALKING) isSkidding = true;
            goto END_HORIZONTAL_VELOCITY_CALCULATION;
        }
        isSkidding = false;

        if (Input::STATE.playerMoveDirection == Input::PLAYER_DIRECTION_STOP) {
            newXVel -= X_DEACCELERATION_RATE_PASSIVE;
            if (newXVel < 0) newXVel = 0;
        }

        else if (Input::STATE.isHoldingRun & (groundBeneath || wasRunningOnJumpStart)) {
            if (newXVel < X_MAX_SPEED_RUNNING) newXVel += X_ACCELERATION_RATE;
            else newXVel -= X_DEACCELERATION_RATE_PASSIVE;
        }
        
        else {
            if (newXVel < X_MAX_SPEED_WALKING) newXVel += X_ACCELERATION_RATE;
            else newXVel -= X_DEACCELERATION_RATE_PASSIVE;
        }

        if (!isFacingRight) newXVel *= -1;

        xVelocity = newXVel;
    }
END_HORIZONTAL_VELOCITY_CALCULATION:


    if (groundBeneath) {

        lastGroundBeneath = GetTime();

        if (!isAscending) {
            // Is on the ground
            hitbox.y = groundBeneath->hitbox.y - hitbox.height;
            yVelocity = 0;
            yVelocityTarget = 0;
        }
    }


    yVelocityWithinTarget =
        abs((int) (yVelocity - yVelocityTarget)) < Y_VELOCITY_TARGET_TOLERANCE;
    
    if (yVelocityWithinTarget) {
        isAscending = false;

        if (!groundBeneath) {
            // Starts falling down
            yVelocityTarget = DOWNWARDS_VELOCITY_TARGET;
        }
    }

    now = GetTime();
    if (!isAscending &&
        (now - lastPressedJump < jumpBufferBackwardsSize()) &&
        (now - lastGroundBeneath < JUMP_BUFFER_FORWARDS_SIZE)) {

        jump();

        // Player hit enemy
        // -- this check is for the case the player jumps off the enemy,
        // and only in the case the enemy wasn't already destroyed in the previous frame.
        if (groundBeneath &&
            groundBeneath->tags & Level::IS_ENEMY) {

            lastGroundBeneath = GetTime();
            EnemyKill(groundBeneath);
            groundBeneath = 0;
        }
    }

    SetPos({ hitbox.x + xVelocity,
                        hitbox.y - yVelocity });


COLISION_CHECKING:
    // Collision checking
    {
        if (hitbox.y + hitbox.height > FLOOR_DEATH_HEIGHT) {
            die();
            return;
        }

        Level::Entity *entity = (Level::Entity *) Level::STATE->listHead;

        while (entity != 0) {

            if ((entity->tags & Level::IS_ENEMY) && !entity->isDead) {

                // Enemy hit player
                if (CheckCollisionRecs(entity->hitbox, upperbody)) {
                    die();
                    break;
                }

                // Player hit enemy
                if (CheckCollisionRecs(entity->hitbox, lowerbody)) {
                    lastGroundBeneath = GetTime();
                    EnemyKill(entity);
                    goto next_entity;
                }
            }

            else if (entity->tags & Level::IS_SCENARIO) {

                // Check for collision with level geometry

                Rectangle collisionRec = GetCollisionRec(entity->hitbox, hitbox);

                if (entity->tags & Level::IS_DANGER &&
                    (collisionRec.width > 0 || collisionRec.height > 0 || groundBeneath == entity)) {
                    
                    // Player hit dangerous level element
                    die();
                    break;
                }

                if (collisionRec.width == 0 || collisionRec.height == 0) goto next_entity;
            
                const bool isAWall = collisionRec.width <= collisionRec.height;
                const bool isACeiling = (collisionRec.width >= collisionRec.height) &&
                                    (entity->hitbox.y < hitbox.y);


                if (isAWall && collisionRec.width > 0) {

                    const bool isEntitysRightWall = collisionRec.x > entity->hitbox.x; 
                    const bool isEntitysLeftWall = !isEntitysRightWall;

                    const bool isEntityToTheLeft = entity->hitbox.x < hitbox.x;
                    const bool isEntityToTheRight = !isEntityToTheLeft;

                    const bool isPlayerMovingToTheLeft = hitbox.x < oldX;
                    const bool isPlayerMovingToTheRight = hitbox.x > oldX;

                    // So when the player hits the entity to its left,
                    // he can only collide with its left wall.
                    if ((isEntitysRightWall && isEntityToTheLeft && isPlayerMovingToTheLeft) ||
                        (isEntitysLeftWall && isEntityToTheRight && isPlayerMovingToTheRight)) {


                        // if (GAME_STATE->showDebugHUD) Render::PrintSysMessage("Hit wall");

                        hitbox.x = oldX;

                        if (isHooked) hookLaunched->angularVelocity *= -1;

                    }
                }


                if (isACeiling && isAscending) {

                    // if (GAME_STATE->showDebugHUD) Render::PrintSysMessage("Hit ceiling");
                    
                    isAscending = false;
                    hitbox.y = oldY;
                    yVelocity = (yVelocity * -1) * CEILING_VELOCITY_FACTOR;
                    yVelocityTarget = DOWNWARDS_VELOCITY_TARGET;

                    if (isHooked) hookLaunched->angularVelocity *= -1;
                }

                // TODO maybe ground check should be here as well
            }

            else if (entity->tags & Level::IS_EXIT &&
                        CheckCollisionRecs(entity->hitbox, hitbox)) {

                // Player exit level

                Level::GoToOverworld();
            }

            else if (entity->tags & Level::IS_GLIDE &&
                        CheckCollisionRecs(entity->hitbox, hitbox) &&
                        mode != PLAYER_MODE_GLIDE) {
                SetMode(PLAYER_MODE_GLIDE);
                return;
            }

            else if (entity->tags & Level::IS_TEXTBOX &&
                        CheckCollisionRecs(entity->hitbox, hitbox)) {

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


    isGliding = false;
    if (yVelocity > yVelocityTarget && !isHooked) {

        if (mode == PLAYER_MODE_GLIDE &&
                !isAscending &&
                Input::STATE.isHoldingRun) {

            // Is gliding
            yVelocity = Y_VELOCITY_GLIDING;
            isGliding = true;

        } else {

            // Accelerates jump's vertical movement
            yVelocity -= Y_ACCELERATION_RATE;
        }
    }

    if (isHooked) hookLaunched->FollowPlayer();

    if (Render::GetTextboxTextId() != -1 && !collidedWithTextboxButton) {
        Render::DisplayTextboxStop();
        TraceLog(LOG_TRACE, "Textbox stopped displaying.");
    }


    sprite = animationTick();
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
        SetPos(pos);
    } else {
        SetPos(origin);
    }
    isAscending = false;
    isGliding = false;
    yVelocity = 0;
    yVelocityTarget = 0;
    xVelocity = 0;

    if (hookLaunched) delete hookLaunched;

    CameraLevelCentralizeOnPlayer();

    TraceLog(LOG_DEBUG, "Player continue.");
}

void Player::SetCheckpoint() {

    if (!groundBeneath) {
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

    Vector2 pos = RectangleGetPos(hitbox);
    pos.y += hitbox.height / 2;
    Level::STATE->checkpoint = Level::CheckpointAdd(pos);

    Level::STATE->checkpointsLeft--;

    TraceLog(LOG_DEBUG, "Player set checkpoint at x=%.1f, y=%.1f.", pos.x, pos.y);
}

void Player::LaunchGrapplingHook() {

    if (hookLaunched) {
        delete hookLaunched;
        return;
    }

    hookLaunched = GrapplingHook::Initialize();
}

void Player::jump() {

    isAscending = true;
    yVelocity = jumpStartVelocity();
    yVelocityTarget = 0.0f;
    wasRunningOnJumpStart = Input::STATE.isHoldingRun;
    Sounds::Play(SOUNDS->Jump);
}

void Player::die() {

    isDead = true;
    Level::STATE->isPaused = true;

    TraceLog(LOG_DEBUG, "You Died.\n\tx=%f, y=%f, isAscending=%d",
                hitbox.x, hitbox.y, isAscending);
}

// The vertical velocity that works as the initial
// propulsion of a jump
float Player::jumpStartVelocity() {

    // Velocity if player's swinging from a hook
    if (hookLaunched && hookLaunched->attachedTo) {
        
        auto h = hookLaunched;
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
float Player::jumpBufferBackwardsSize() {

    if (isGliding)
        return JUMP_BUFFER_BACKWARDS_SIZE_GLIDING;
    else
        return JUMP_BUFFER_BACKWARDS_SIZE;
}


void Player::createAnimations() {

    animationInPlace.stills.push_back(Animation::Still{ &SPRITES->PlayerDefault, 1 });

    animationWalking.stills.push_back(Animation::Still({ &SPRITES->PlayerWalking1, 8 })); // TODO create a define for this value
    animationWalking.stills.push_back(Animation::Still({ &SPRITES->PlayerDefault, 8 }));
    animationWalking.stills.push_back(Animation::Still({ &SPRITES->PlayerWalking2, 8 }));
    animationWalking.stills.push_back(Animation::Still({ &SPRITES->PlayerDefault, 8 }));

    animationRunning.stills.push_back(Animation::Still({ &SPRITES->PlayerRunning1, 12 }));
    animationRunning.stills.push_back(Animation::Still({ &SPRITES->PlayerRunning2, 12 }));

    animationSkidding.stills.push_back(Animation::Still({ &SPRITES->PlayerSkidding, 1 }));

    animaitonJumpingUp.stills.push_back(Animation::Still({ &SPRITES->PlayerJumpingUp, 1 }));

    animationJumpingDown.stills.push_back(Animation::Still({ &SPRITES->PlayerJumpingDown, 1 }));

    animationGlideWalking.stills.push_back(Animation::Still({ &SPRITES->PlayerGlideOn, 1 }));

    animationGlideFalling.stills.push_back(Animation::Still({ &SPRITES->PlayerGlideFalling, 1 }));

    animationSwinging.stills.push_back(Animation::Still({ &SPRITES->PlayerSwinging, 1 }));
}

Animation::Animation *Player::getCurrentAnimation() {

    Animation::Animation *animation;


    if (mode == PLAYER_MODE_GLIDE) {
        if (isGliding)
            animation =         &animationGlideFalling;
        else
            animation =         &animationGlideWalking;
    }

    else if (PLAYER->hookLaunched && PLAYER->hookLaunched->attachedTo)
        animation =             &animationSwinging;

    else if (!groundBeneath && PLAYER->isAscending)
        animation =             &animaitonJumpingUp;
    else if (!groundBeneath && !PLAYER->isAscending)
        animation =             &animationJumpingDown;

    else if (((Input::STATE.playerMoveDirection == Input::PLAYER_DIRECTION_LEFT && xVelocity > 0) ||
                (Input::STATE.playerMoveDirection == Input::PLAYER_DIRECTION_RIGHT && xVelocity < 0)) &&
                isSkidding)
        animation =             &animationSkidding;

    else if (groundBeneath && abs(xVelocity) > 0.5 && abs(xVelocity) < 6.5f) // TODO create a default for this value
        animation =             &animationWalking;

    else if (groundBeneath && abs(xVelocity) >= 6.5f) // TODO create a default for this value
        animation =             &animationRunning;

    else animation =            &animationInPlace;


    return animation;
}
