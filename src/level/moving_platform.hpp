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

    Rectangle GetOriginHitbox() override {
        // Despite not really having the concept of origin, it still offers an origin hitbox
        // because parts of the code use them to interact with dead entities (i.e. isDead = true).
        return hitbox;
    }

    void SetPos(Vector2 pos);

    void SetHitboxPos(Vector2 pos) override {
        SetPos({ pos.x + hitbox.width/2, pos.y + hitbox.height/2 });
    }

    void SetOrigin(Vector2 origin) override {
        // Does nothing. DANGEROUS!
        (void) origin;
    }

    void Draw() override {
        Vector2 p = PosInSceneToScreen(pos);
        if (EDITOR_STATE->isEnabled) DrawCircleLines(p.x, p.y, 20, color);
    }

    void DrawMoveGhost() override {
        Vector2 p = PosInSceneToScreen(EditorEntitySelectionCalcMove(pos));
        DrawCircleLines(p.x, p.y, 20, { color.r, color.g, color.b, EDITOR_SELECTION_MOVE_TRANSPARENCY });
    }
};


class MovingPlatform : public Level::Entity {

public:

    MovingPlatformAnchor startAnchor, endAnchor;


    MovingPlatform() : Level::Entity(), startAnchor(this, GREEN), endAnchor(this, RED) {};

    static MovingPlatform *Add();

    static MovingPlatform *Add(Vector2 startPos, Vector2 endPos, int size);

    static void CheckAndAdd(Vector2 origin);

    Rectangle GetOriginHitbox() override {
        return {
            origin.x - (hitbox.width/2), origin.y - (hitbox.height/2),
            hitbox.width, hitbox.height
        };
    }

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
