#pragma once


#include <raylib.h>

#include "level.hpp"


class MovingPlatform : public Level::Entity {

public:

    static MovingPlatform *Add();

    static void CheckAndAdd(Vector2 origin);

    void Tick() override;

    void Draw() override;

    std::string PersistanceSerialize() override;
    void PersistenceParse(const std::string &data) override;


private:

    Vector2 startPos, endPos, currentPos;
    Rectangle startHitbox, endHitbox;
    int size; // in blocks
    float angle;


    void setStart(Vector2 point);

    void setEnd(Vector2 point);

    // Resizes platform (in blocks)
    void setSize(int size);

    // Updates platform hitbox based on size and currentPoint 
    void updateHitbox();
};
