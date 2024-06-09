#ifndef _PLAYER_H_INCLUDED_
#define _PLAYER_H_INCLUDED_


#include <raylib.h>

#include "level.hpp"
#include "grappling_hook.hpp"
#include "textbox.hpp"
#include "../animation.hpp"

#define PLAYER_PERSISTENCE_ID   "player"


typedef enum PlayerMode {
    PLAYER_MODE_DEFAULT,
    PLAYER_MODE_GLIDE
} PlayerMode;


class Player : public Level::Entity, private Animation::IAnimated {

public:
    // The ground beneath the player, updated every frame, or 0 if there's no ground beneath
    Level::Entity *groundBeneath;
    Level::Entity *groundBeneathLastFrame;

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

    // for jump buffers
    double lastPressedJump;
    double lastGroundBeneathTime;
    double lastLandedOnGroundTime;
    Level::Entity *lastGroundBeneath;

    // A reference to the launched grappling hook, if there's one  
    GrapplingHook *hookLaunched;

    // A reference to the moving platform the player is on, if there's one
    Level::Entity *movingPlatformBeneath;

    // Controls textbox toggling
    Textbox *textboxCollidedLastFrame;

    int jumpGlowAnimationCountdown;
    int jumpGlowStrength;

    // Initializes and adds the player to the level
    static Player *Initialize();

    // Initializes and adds the player to the level in the given origin
    static Player *Initialize(Vector2 origin);

    // Sets the player's origin based on pos, if there aren't other things there already.
    void CheckAndSetOrigin(Vector2 pos);

    // Moves the player to pos, if there aren't other things there already.
    void CheckAndSetPos(Vector2 pos);

    void SetHitbox(Rectangle hitbox);

    // Moves the player to pos, updating the collision hitboxes in the proccess
    void SetHitboxPos(Vector2 pos) override;

    void SetMode(PlayerMode mode);

    void Jump();

    void Tick();

    void Draw() override;

    // Continues the game after dying.
    void Continue();

    void SetCheckpoint();

    void LaunchGrapplingHook();

    void PersistenceParse(const std::string &data) override;


private:

    static Animation::Animation animationInPlace;
    static Animation::Animation animationWalking;
    static Animation::Animation animationRunning;
    static Animation::Animation animationSkidding;
    static Animation::Animation animaitonJumpingUp;
    static Animation::Animation animationJumpingDown;
    static Animation::Animation animationGlideInPlace;
    static Animation::Animation animationGlideGliding;
    static Animation::Animation animationSwinging;
    static Animation::Animation animationSwingingForwards;
    static Animation::Animation animationSwingingBackwards;


    // TODO turn this into a state machine
    bool isSkidding;


    void jump();

    void jumpDouble();

    void die();

    float jumpStartVelocity();

    float jumpBufferBackwardsSize();

    void createAnimations();

    /*
        Draws glow effect on the player's feet when jumping
        @param strength The strength of the effect (from 0 to 3)
        @param progression From far into the animation it is (from 0 to 1)
    */
    void drawJumpGlow(int strength, float progression);
    
    Animation::Animation *getCurrentAnimation();

    std::string EntityTypeID() {
        return PLAYER_PERSISTENCE_ID;
    }
};


// Reference to the player entity, part of the level entity list
extern Player *PLAYER;


#endif // _PLAYER_H_INCLUDED_
