#include <string>
#include <math.h>

#include "moving_platform.hpp"
#include "level.hpp"
#include "../camera.hpp"
#include "../editor.hpp"
#include "../render.hpp"


#define PLATFORM_SPEED      2


MovingPlatform *MovingPlatform::Add() {
    return Add({ 0, 0 }, { 20, 20 }, 3);
}

MovingPlatform *MovingPlatform::Add(Vector2 startPos, Vector2 endPos, int size) {


    MovingPlatform *newPlatform = new MovingPlatform();

    newPlatform->tags = Level::IS_SCENARIO +
                            Level::IS_GROUND +
                            Level::IS_HOOKABLE +
                            Level::IS_MOVING_PLATFORM;
    newPlatform->sprite = &SPRITES->MovingPlatform;
    newPlatform->isFacingRight = true;
    newPlatform->layer = -1;

    newPlatform->setStartPos(startPos);
    newPlatform->setEndPos(endPos);
    newPlatform->setSize(size);


    LinkedList::AddNode(&Level::STATE->listHead, newPlatform);

    TraceLog(LOG_TRACE, "Added moving platform to level (x=%.1f, y=%.1f)",
                newPlatform->hitbox.x, newPlatform->hitbox.y);


    return newPlatform;
}

void MovingPlatform::CheckAndAdd(Vector2 origin) {

    // TODO check for collisions

    Add(origin, { origin.x + 180, origin.y - 180 }, 3);
}

void MovingPlatform::Tick() {

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
        DrawTexture(sprite->sprite, pos.x, pos.y, WHITE);
    }

    // Draw platform origin ghost
    if (EDITOR_STATE->isEnabled) {
        for (int i = 0; i < size; i++) {
            Vector2 pos = PosInSceneToScreen({
                                            GetOriginHitbox().x + (sprite->sprite.width * sprite->scale * i),
                                            GetOriginHitbox().y });
            DrawTexture(sprite->sprite, pos.x, pos.y, { WHITE.r, WHITE.g, WHITE.b, ORIGIN_GHOST_TRANSPARENCY });
        }
    }

    // Draw anchors
    startAnchor.Draw();
    endAnchor.Draw();
}

void MovingPlatform::DrawMoveGhost() {
    for (int i = 0; i < size; i++) {
        Vector2 pos = PosInSceneToScreen({
                                        hitbox.x + (sprite->sprite.width * sprite->scale * i),
                                        hitbox.y });
        DrawTexture(sprite->sprite, pos.x, pos.y, { WHITE.r, WHITE.g, WHITE.b, EDITOR_SELECTION_MOVE_TRANSPARENCY });
    }
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

    setStartPos(startPos);
    setEndPos(endPos);
    setSize(size);
}

void MovingPlatform::setStartPos(Vector2 startPos) {

    startAnchor.SetPos(startPos);
    this->origin = this->startAnchor.pos;
    updateAngle();
    movePlatformTo(this->origin); // resets current position
}

void MovingPlatform::setEndPos(Vector2 endPos) {

    endAnchor.SetPos(endPos);
    updateAngle();
    movePlatformTo(this->origin); // resets current position
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
