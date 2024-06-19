#pragma once


#include <raylib.h>

#include "level.hpp"
#include "../animation.hpp"

#define CHECKPOINT_PICKUP_ENTITY_ID        "checkpoint_pickup"


class CheckpointPickup : public Level::Entity, private Animation::IAnimated {

public:

    bool wasPickedUp;


    static Level::Entity *AddFromPersistence();

    // Adds a level checkpoint pickup in the given position
    static Level::Entity *Add(Vector2 pos);

    // Initializes and adds a checkpoint pickup to the level in the given origin,
    // if there are no other entities there already
    static void AddFromEditor(Vector2 pos, int interactionTags);

    void Tick();

    void Draw();

private:

    static Animation::Animation animation;


    void createAnimations();
    
    Animation::Animation *getCurrentAnimation();
};
