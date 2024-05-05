#include <string>
#include <math.h>

#include "moving_platform.hpp"
#include "level.hpp"
#include "../camera.hpp"
#include "../render.hpp"


#define PLATFORM_SPEED          2

#define DEFAULT_SIZE            4

#define ANCHOR_HITBOX_SIDE      40



MovingPlatformAnchor::MovingPlatformAnchor(MovingPlatform *parent, Color color) {
    this->parent = parent;
    this->color = color;
    tags = Level::IS_ANCHOR;
}

Rectangle MovingPlatformAnchor::GetOriginHitbox() {

    // Despite not really having the concept of origin, it still offers an origin hitbox
    // because parts of the code use them to interact with dead entities (i.e. isDead = true).

    return hitbox;
}

void MovingPlatformAnchor::SetPos(Vector2 pos) {

    Dimensions d = { ANCHOR_HITBOX_SIDE, ANCHOR_HITBOX_SIDE };

    this->pos = pos;
    this->hitbox =  {
        this->pos.x - d.width/2,
        this->pos.y - d.height/2,
        d.width,
        d.height,
    };

    parent->UpdateAfterAnchorMove();
}

void MovingPlatformAnchor::SetHitboxPos(Vector2 pos) {
    SetPos({ pos.x + hitbox.width/2, pos.y + hitbox.height/2 });
}

void MovingPlatformAnchor::SetOrigin(Vector2 origin) {
    
    // Does nothing. DANGEROUS!
    (void) origin;
}

void MovingPlatformAnchor::Draw() {

    if (EDITOR_STATE->isEnabled) {
        Vector2 p = PosInSceneToScreen(pos);
        float r = ScaleInSceneToScreen(ANCHOR_HITBOX_SIDE / 2);
        DrawCircleLines(p.x, p.y, r, color);
    }
}

void MovingPlatformAnchor::DrawMoveGhost() {

    Vector2 p = PosInSceneToScreen(EditorEntitySelectionCalcMove(pos));
    float r = ScaleInSceneToScreen(ANCHOR_HITBOX_SIDE / 2);

    DrawCircleLines(p.x, p.y, r,
                    { color.r, color.g, color.b, EDITOR_SELECTION_MOVE_TRANSPARENCY });
}

///////

MovingPlatform *MovingPlatform::Add() {
    return Add({ 0, 0 }, { 20, 20 }, DEFAULT_SIZE);
}

MovingPlatform *MovingPlatform::Add(Vector2 startPos, Vector2 endPos, int size) {


    MovingPlatform *newPlatform = new MovingPlatform();

    newPlatform->tags = Level::IS_COLLIDE_WALL +
                            Level::IS_GROUND +
                            Level::IS_HOOKABLE +
                            Level::IS_MOVING_PLATFORM +
                            Level::IS_PERSISTABLE;
    newPlatform->sprite = &SPRITES->MovingPlatform;
    newPlatform->isFacingRight = true;
    newPlatform->layer = -1;

    newPlatform->startAnchor.SetPos(startPos);
    newPlatform->endAnchor.SetPos(endPos);
    newPlatform->setSize(size);


    LinkedList::AddNode(&Level::STATE->listHead, newPlatform);

    TraceLog(LOG_TRACE, "Added moving platform to level (x=%.1f, y=%.1f)",
                newPlatform->hitbox.x, newPlatform->hitbox.y);


    return newPlatform;
}

void MovingPlatform::CheckAndAdd(Vector2 origin) {

    // TODO check for collisions

    Add(origin, { origin.x + 140, origin.y - 90 }, DEFAULT_SIZE);
}

Rectangle MovingPlatform::GetOriginHitbox() {
    
    return {
        origin.x - (hitbox.width/2), origin.y - (hitbox.height/2),
        hitbox.width, hitbox.height
    };
}

void MovingPlatform::UpdateAfterAnchorMove() {

    this->origin = this->startAnchor.pos;
    updateAngle();
    movePlatformTo(origin); // resets current position
}

void MovingPlatform::Tick() {

    Vector2 oldPos = currentPos;

    int direction = isFacingRight ? 1 : -1;
    
    movePlatformTo({
        (float)(currentPos.x + PLATFORM_SPEED * cos(angle) * direction),
        (float)(currentPos.y - PLATFORM_SPEED * sin(angle) * direction)
    });

    if ((currentPos.x > startAnchor.pos.x && currentPos.x > endAnchor.pos.x) ||
        (currentPos.x < startAnchor.pos.x && currentPos.x < endAnchor.pos.x) ||
        (currentPos.y > startAnchor.pos.y && currentPos.y > endAnchor.pos.y) ||
        (currentPos.y < startAnchor.pos.y && currentPos.y < endAnchor.pos.y)) {

            isFacingRight = !isFacingRight;
    }


    lastFrameTrajectory = {
        currentPos.x - oldPos.x,
        currentPos.y - oldPos.y
    };
}

void MovingPlatform::Draw() {

    // Draw track
    Vector2 sp = PosInSceneToScreen(startAnchor.pos);
    Vector2 ep = PosInSceneToScreen(endAnchor.pos);
    DrawLine(sp.x, sp.y, ep.x, ep.y, RAYWHITE);

    // Draw platform
    for (int i = 0; i < size; i++) {
        Vector2 pos = PosInSceneToScreen({
                                        hitbox.x + (sprite->sprite.width * sprite->scale * i),
                                        hitbox.y });
        Render::DrawTexture(sprite, pos, WHITE, 0, false);
    }

    // Draw platform origin ghost
    if (EDITOR_STATE->isEnabled) {
        for (int i = 0; i < size; i++) {
            Vector2 pos = PosInSceneToScreen({
                                            GetOriginHitbox().x + (sprite->sprite.width * sprite->scale * i),
                                            GetOriginHitbox().y });
            Color color = { WHITE.r, WHITE.g, WHITE.b, ORIGIN_GHOST_TRANSPARENCY };
            Render::DrawTexture(sprite, pos, color, 0, false);
        }
    }

    // Draw anchors
    startAnchor.Draw();
    endAnchor.Draw();
}

std::string MovingPlatform::PersistanceSerialize() {

    std::string data = Level::Entity::PersistanceSerialize();
    persistanceAddValue(&data, "startPosX", std::to_string(startAnchor.pos.x));
    persistanceAddValue(&data, "startPosY", std::to_string(startAnchor.pos.y));
    persistanceAddValue(&data, "endPosX", std::to_string(endAnchor.pos.x));
    persistanceAddValue(&data, "endPosY", std::to_string(endAnchor.pos.y));
    persistanceAddValue(&data, "size", std::to_string(size));
    return data;
}

void MovingPlatform::PersistenceParse(const std::string &data) {

    Vector2 startPos, endPos;
    int size;

    Level::Entity::PersistenceParse(data);
    startPos.x = std::stof(persistenceReadValue(data, "startPosX"));
    startPos.y = std::stof(persistenceReadValue(data, "startPosY"));
    endPos.x = std::stof(persistenceReadValue(data, "endPosX"));
    endPos.y = std::stof(persistenceReadValue(data, "endPosY"));
    size = std::stoi(persistenceReadValue(data, "size"));

    startAnchor.SetPos(startPos);
    endAnchor.SetPos(endPos);
    setSize(size);
}

void MovingPlatform::setSize(int size) {

    if (size < 1) {
        TraceLog(LOG_WARNING, "Tried to set Moving Platform size to %d (<1)", size);
        size = 1;
    }

    this->size = size;

    updateHitbox();
}

void MovingPlatform::movePlatformTo(Vector2 newPos) {
    this->currentPos = newPos;
    updateHitbox();
}

void MovingPlatform::updateHitbox() {

    Dimensions dimensions = SpriteScaledDimensions(sprite);
    dimensions.width *= size; // only resizes platform horizontally

    hitbox = {
        currentPos.x - dimensions.width/2,
        currentPos.y - dimensions.height/2,
        dimensions.width,
        dimensions.height
    };
}

void MovingPlatform::updateAngle() {
    angle = atan2(startAnchor.pos.y - endAnchor.pos.y, endAnchor.pos.x - startAnchor.pos.x);
}
