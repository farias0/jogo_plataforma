#pragma once


#include <raylib.h>

#include "level.hpp"
#include "../animation.hpp"


class CheckpointPickup : public Level::Entity, private Animation::IAnimated {

public:

    bool wasPickedUp;


    static Level::Entity *Add();

    // Adds a level checkpoint pickup in the given position
    static Level::Entity *Add(Vector2 pos);

    // Initializes and adds a checkpoint pickup to the level in the given origin,
    // if there are no other entities there already
    static void CheckAndAdd(Vector2 pos);

    void Tick();

    void Draw();

private:

    static Animation::Animation animation;


    void createAnimations();
    
    Animation::Animation *getCurrentAnimation();
};
