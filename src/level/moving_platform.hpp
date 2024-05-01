#pragma once

#include <raylib.h>

#include "level.hpp"
#include "../editor.hpp"
#include "../camera.hpp"


#define MOVING_PLATFORM_PERSISTENCE_ID          "moving_platform"


class MovingPlatform;

/*
    Defines the two ends of a moving platform's tracks
*/
// TODO consider making the interface IAnchor, if other entities use a similar concept
class MovingPlatformAnchor : public Level::Entity {

public:

    MovingPlatform *parent;
    Vector2 pos;
    Color color;


    MovingPlatformAnchor(MovingPlatform *parent, Color color);

    Rectangle GetOriginHitbox() override;

    void SetPos(Vector2 pos);

    void SetHitboxPos(Vector2 pos) override;

    void SetOrigin(Vector2 origin) override;

    void Draw() override;

    void DrawMoveGhost() override;
};


class MovingPlatform : public Level::Entity {

public:

    MovingPlatformAnchor startAnchor, endAnchor;

    // So other entities can move together with this platform
    Vector2 lastFrameTrajectory;


    MovingPlatform() : Level::Entity(), startAnchor(this, GREEN), endAnchor(this, RED) {};

    static MovingPlatform *Add();

    static MovingPlatform *Add(Vector2 startPos, Vector2 endPos, int size);

    static void CheckAndAdd(Vector2 origin);

    Rectangle GetOriginHitbox() override;

    // Recalculates parameters after one of its Anchors have moved
    void UpdateAfterAnchorMove();

    void Tick() override;

    void Draw() override;

    std::string PersistanceSerialize() override;
    void PersistenceParse(const std::string &data) override;
    std::string PersistanceEntityID() {
        return MOVING_PLATFORM_PERSISTENCE_ID;
    }

private:

    Vector2 currentPos; // Middle point of the platform
    int size; // In blocks
    float angle; // In radians


    // Resizes platform (in blocks)
    void setSize(int size);

    // Moves platform to a new position
    void movePlatformTo(Vector2 newPos);

    // Updates platform hitbox based on size and currentPoint 
    void updateHitbox();

    // Updates angle according to start and end positions
    void updateAngle();
};
