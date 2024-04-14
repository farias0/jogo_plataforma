#ifndef _PLAYER_H_INCLUDED_
#define _PLAYER_H_INCLUDED_


#include <raylib.h>

#include "level.hpp"
#include "grappling_hook.hpp"
#include "../animation.hpp"


typedef enum PlayerMode {
    PLAYER_MODE_DEFAULT,
    PLAYER_MODE_GLIDE
} PlayerMode;


class Player : public Level::Entity, private Animation::IAnimated {

public:
    // The ground beneath the player, updated every frame, or 0 if there's no ground beneath
    Level::Entity *groundBeneath;

    // TODO maybe add multiple hitboxes support for every entity
    Rectangle upperbody, lowerbody;

    // If the player is on the "going up" phase of the jump
    bool isAscending;

    // If the player is on mode 'GLIDE' and is actively gliding
    bool isGliding;

    float yVelocity;
    float yVelocityTarget;
    float xVelocity;

    // If the player is jumping, if he was running at the jump's start
    bool wasRunningOnJumpStart;

    PlayerMode mode;

    // timestamps, for jump buffers
    double lastPressedJump;
    double lastGroundBeneath;

    // A reference to the launched grappling hook, if there's one  
    GrapplingHook *hookLaunched;


    // Initializes and adds the player to the level in the given origin
    static void Initialize(Vector2 origin);

    // Sets the player's origin based on pos, if there aren't other things there already.
    void CheckAndSetOrigin(Vector2 pos);

    // Moves the player to pos, if there aren't other things there already.
    void CheckAndSetPos(Vector2 pos);

    void SetHitbox(Rectangle hitbox);

    // Moves the player to pos, updating the collision hitboxes in the proccess
    void SetPos(Vector2 pos);

    void SetMode(PlayerMode mode);

    void Jump();

    void Tick();

    // Continues the game after dying.
    void Continue();

    void SetCheckpoint();

    void LaunchGrapplingHook();


private:

    Animation::Animation animationInPlace;
    Animation::Animation animaitonWalking;
    Animation::Animation animaitonJumpingUp;
    Animation::Animation animationGlideWalking;
    Animation::Animation animationGlideFalling;
    Animation::Animation animationSwinging;


    void jump();

    void die();

    float jumpStartVelocity();

    float jumpBufferBackwardsSize();

    void createAnimations();
    Animation::Animation *getCurrentAnimation();
};


// Reference to the player entity, part of the level entity list
extern Player *PLAYER;


#endif // _PLAYER_H_INCLUDED_
