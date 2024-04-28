#include <string>

#include "moving_platform.hpp"
#include "level.hpp"
#include "../camera.hpp"


#define PLATFORM_SPEED      20


MovingPlatform *MovingPlatform::Add() {


    MovingPlatform *newPlatform = new MovingPlatform();

    newPlatform->tags = Level::IS_SCENARIO +
                            Level::IS_GROUND +
                            Level::IS_HOOKABLE +
                            Level::IS_MOVING_PLATFORM;
    newPlatform->sprite = &SPRITES->Block;
    newPlatform->isFacingRight = true;
    newPlatform->layer = -1;

    // TODO
    newPlatform->size = 3; 
    newPlatform->origin = { 0, 0 };
    newPlatform->startPos = newPlatform->origin;
    newPlatform->endPos = { newPlatform->startPos.x + 180, newPlatform->endPos.y + 180 };
    newPlatform->currentPos = newPlatform->startPos;
    newPlatform->updateHitbox();


    LinkedList::AddNode(&Level::STATE->listHead, newPlatform);

    TraceLog(LOG_TRACE, "Added moving platform to level (x=%.1f, y=%.1f)",
                newPlatform->hitbox.x, newPlatform->hitbox.y);


    return newPlatform;
}

void MovingPlatform::CheckAndAdd(Vector2 origin) {

    // TODO check for collisions

    MovingPlatform *newPlatform = Add();
    newPlatform->origin = origin;
    newPlatform->startPos = newPlatform->origin;
    newPlatform->endPos = { newPlatform->startPos.x + 180, newPlatform->startPos.y + 180 };
    newPlatform->currentPos = newPlatform->startPos;
    newPlatform->updateHitbox();

}

void MovingPlatform::Tick() {

    // Move it
    
    // probably will need the track's angle

}

void MovingPlatform::Draw() {

    // Draw tracks
    Vector2 sp = PosInSceneToScreen(startPos);
    Vector2 ep = PosInSceneToScreen(endPos);
    DrawLine(sp.x, sp.y, ep.x, ep.y, RAYWHITE);

    // Draw platform
    for (int i = 0; i < size; i++) {
        Vector2 pos = PosInSceneToScreen({
                                        hitbox.x + (sprite->sprite.width * sprite->scale * i),
                                        hitbox.y });
        DrawTexture(sprite->sprite, pos.x, pos.y, WHITE);
    }

}

std::string MovingPlatform::PersistanceSerialize() {

    std::string data = Level::Entity::PersistanceSerialize();
    // persistanceAddValue(&data, "textId", std::to_string(textId));
    return data;
}

void MovingPlatform::PersistenceParse(const std::string &data) {

    Level::Entity::PersistenceParse(data);
    // textId = std::stoi(persistenceReadValue(data, "textId"));
}

void MovingPlatform::setSize(int size) {

    if (size < 1) {
        TraceLog(LOG_WARNING, "Tried to set Moving Platform size to %d (<1)", size);
        size = 1;
    }

    // TODO check for collisions before

    this->size = size;
    updateHitbox();
}

void MovingPlatform::updateHitbox() {

    Dimensions dimensions = SpriteScaledDimensions(sprite);
    dimensions.width *= size;

    hitbox = {
        currentPos.x - dimensions.width/2,
        currentPos.y - dimensions.height/2,
        dimensions.width,
        dimensions.height
    };
}
