#include <raylib.h>

#include "powerups.hpp"
#include "level.hpp"
#include "../editor.hpp"


Level::Entity *GlideAddFromPersistence() {
    return GlideAdd({ 0, 0 });
}

Level::Entity *GlideAdd(Vector2 origin) {
    
    Level::Entity *glide = new Level::Entity();

    glide->tags = Level::IS_GLIDE_PICKUP +
                    Level::IS_PERSISTABLE;
    glide->origin = origin;
    glide->sprite = &SPRITES->GlideItem;
    glide->hitbox = SpriteHitboxFromEdge(glide->sprite, glide->origin);

    glide->entityTypeID = GLIDE_PICKUP_ENTITY_ID;

    LinkedList::AddNode(&Level::STATE->listHead, glide);

    TraceLog(LOG_TRACE, "Added glide item to level (x=%.1f, y=%.1f)",
                glide->hitbox.x, glide->hitbox.y);

    return glide;
}  

void GlideAddFromEditor(Vector2 origin, int interactionTags) {

    if (!(interactionTags & EDITOR_INTERACTION_CLICK)) return;


    origin = SnapToGrid(origin, LEVEL_GRID);

    Rectangle hitbox = SpriteHitboxFromEdge(&SPRITES->GlideItem, origin);
    if (Level::CheckCollisionWithAnything(hitbox)) return;

    if (!Level::GetGroundBeneathHitbox(hitbox)) {
        TraceLog(LOG_DEBUG, "Didn't add glide item to level, no ground beneath");
        return;
    }
    
    GlideAdd(origin);
}
