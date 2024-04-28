#pragma once


#include <raylib.h>

#include "level.hpp"


class MovingPlatform : public Level::Entity {

public:

    static MovingPlatform *Add();

    static MovingPlatform *Add(Vector2 startPos, Vector2 endPos, int size);

    static void CheckAndAdd(Vector2 origin);

    void Tick() override;

    void Draw() override;

    std::string PersistanceSerialize() override;
    void PersistenceParse(const std::string &data) override;


private:

    Vector2 startPos, endPos, currentPos;
    Rectangle startHitbox, endHitbox; // Hitboxes for the start and end position
    int size; // In blocks
    float angle; // In radians


    void setStartPos(Vector2 startPos);

    void setEndPos(Vector2 endPos);

    // Resizes platform (in blocks)
    void setSize(int size);

    // Moves platform to a new position
    void movePlatformTo(Vector2 newPos);

    // Updates platform hitbox based on size and currentPoint 
    void updateHitbox();

    // Updates angle according to start and end positions
    void updateAngle();
};
