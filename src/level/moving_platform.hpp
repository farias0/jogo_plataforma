#pragma once


#include <raylib.h>

#include "level.hpp"
#include "../editor.hpp"
#include "../camera.hpp"


class MovingPlatform;

/*
    Defines the two ends of a moving platform's tracks
*/
class MovingPlatformAnchor : public Level::Entity {

public:

    MovingPlatform *parent;
    Vector2 pos;
    Color color;

    MovingPlatformAnchor(MovingPlatform *parent, Color color) {
        this->parent = parent;
        this->color = color;
        tags = Level::IS_MOVING_PLATFORM;
    }

    void SetPos(Vector2 pos) {
        Dimensions d = { 40, 40 };
        this->pos = pos;
        this->hitbox =  {
            this->pos.x - d.width/2,
            this->pos.y - d.height/2,
            d.width,
            d.height,
        };
    }

    void Draw() override {
        Vector2 p = PosInSceneToScreen(pos);
        if (EDITOR_STATE->isEnabled) DrawCircleLines(p.x, p.y, 20, color);
    }

    void DrawMoveGhost() override {
        Vector2 p = PosInSceneToScreen(pos);
        DrawCircleLines(p.x, p.y, 20, { color.r, color.g, color.b, EDITOR_SELECTION_MOVE_TRANSPARENCY });
    }
};


class MovingPlatform : public Level::Entity {

public:

    MovingPlatformAnchor startAnchor, endAnchor;


    MovingPlatform() : startAnchor(this, GREEN), endAnchor(this, RED), Level::Entity() {};

    static MovingPlatform *Add();

    static MovingPlatform *Add(Vector2 startPos, Vector2 endPos, int size);

    static void CheckAndAdd(Vector2 origin);

    Rectangle GetOriginHitbox() override {
        return {
            origin.x - (hitbox.width/2), origin.y - (hitbox.height/2),
            hitbox.width, hitbox.height
        };
    }

    void Tick() override;

    void Draw() override;
    void DrawMoveGhost() override;

    std::string PersistanceSerialize() override;
    void PersistenceParse(const std::string &data) override;


private:

    Vector2 currentPos;
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
