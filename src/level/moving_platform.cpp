#include <string>
#include <math.h>

#include "moving_platform.hpp"
#include "level.hpp"
#include "../camera.hpp"
#include "../editor.hpp"


#define PLATFORM_SPEED      2


MovingPlatform *MovingPlatform::Add() {


    MovingPlatform *newPlatform = new MovingPlatform();

    newPlatform->tags = Level::IS_SCENARIO +
                            Level::IS_GROUND +
                            Level::IS_HOOKABLE +
                            Level::IS_MOVING_PLATFORM;
    newPlatform->sprite = &SPRITES->MovingPlatform;
    newPlatform->isFacingRight = true;
    newPlatform->layer = -1;

    // TODO
    newPlatform->origin = { 0, 0 };
    newPlatform->startPos = newPlatform->origin;
    newPlatform->endPos = { newPlatform->startPos.x - 180, newPlatform->endPos.y + 180 };
    newPlatform->currentPos = newPlatform->startPos;
    newPlatform->setSize(3);


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
    newPlatform->endPos = { newPlatform->startPos.x - 180, newPlatform->startPos.y - 180 };
    newPlatform->currentPos = newPlatform->startPos;
    newPlatform->updateHitbox();

}

void MovingPlatform::Tick() {

    // Move it
    
    // probably will need the track's angle
    angle = atan2(startPos.y - endPos.y, endPos.x - startPos.x); // TODO calculate when setting pos

    int direction = isFacingRight ? 1 : -1;
    
    currentPos.x += PLATFORM_SPEED * cos(angle) * direction;
    currentPos.y -= PLATFORM_SPEED * sin(angle) * direction;

    if ((currentPos.x > startPos.x && currentPos.x > endPos.x) ||
        (currentPos.x < startPos.x && currentPos.x < endPos.x) ||
        (currentPos.y > startPos.y && currentPos.y > endPos.y) ||
        (currentPos.y < startPos.y && currentPos.y < endPos.y)) {

            isFacingRight = !isFacingRight;
        }

    updateHitbox();
}

void MovingPlatform::Draw() {

    // Draw tracks
    Vector2 sp = PosInSceneToScreen(startPos);
    Vector2 ep = PosInSceneToScreen(endPos);
    DrawLine(sp.x, sp.y, ep.x, ep.y, RAYWHITE);
    if (EDITOR_STATE->isEnabled) {
        Vector2 s = PosInSceneToScreen(startPos), e = PosInSceneToScreen(endPos);
        DrawCircleLines(s.x, s.y, 20, GREEN);
        DrawCircleLines(e.x, e.y, 20, RED);
    }

    // Draw platform
    //Vector2 c = PosInSceneToScreen(currentPos);
    //DrawCircleLines(c.x, c.y, 20, RAYWHITE);
    if (!EDITOR_STATE->isEnabled) {
        for (int i = 0; i < size; i++) {
            Vector2 pos = PosInSceneToScreen({
                                            hitbox.x + (sprite->sprite.width * sprite->scale * i),
                                            hitbox.y });
            DrawTexture(sprite->sprite, pos.x, pos.y, WHITE);
        }
    }

    // TODO draw ghosts

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

void MovingPlatform::setStart(Vector2 point) {
    Dimensions d = SpriteScaledDimensions(sprite);
    startPos = point;
    startHitbox = {
        startPos.x - d.width/2,
        startPos.y - d.height/2,
        d.width,
        d.height,
    };
}

void MovingPlatform::setEnd(Vector2 point) {
    Dimensions d = SpriteScaledDimensions(sprite);
    endPos = point;
    endHitbox = {
        endPos.x - d.width/2,
        endPos.y - d.height/2,
        d.width,
        d.height,
    };
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
