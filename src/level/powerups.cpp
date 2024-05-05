#include <raylib.h>

#include "powerups.hpp"
#include "level.hpp"


Level::Entity *GlideAdd() {
    return GlideAdd({ 0, 0 });
}

Level::Entity *GlideAdd(Vector2 origin) {
    
    Level::Entity *glide = new Level::Entity();

    glide->tags = Level::IS_GLIDE_PICKUP +
                    Level::IS_PERSISTABLE;
    glide->origin = origin;
    glide->sprite = &SPRITES->GlideItem;
    glide->hitbox = SpriteHitboxFromEdge(glide->sprite, glide->origin);

    glide->persistanceEntityID = GLIDE_PICKUP_PERSISTENCE_ID;

    LinkedList::AddNode(&Level::STATE->listHead, glide);

    TraceLog(LOG_TRACE, "Added glide item to level (x=%.1f, y=%.1f)",
                glide->hitbox.x, glide->hitbox.y);

    return glide;
}  

void GlideCheckAndAdd(Vector2 origin) {

    origin = SnapToGrid(origin, LEVEL_GRID);

    Rectangle hitbox = SpriteHitboxFromEdge(&SPRITES->GlideItem, origin);
    if (Level::CheckCollisionWithAnything(hitbox)) return;

    if (!Level::GetGroundBeneathHitbox(hitbox)) {
        TraceLog(LOG_DEBUG, "Didn't add glide item to level, no ground beneath");
        return;
    }
    
    GlideAdd(origin);
}
