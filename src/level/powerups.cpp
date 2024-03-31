#include <raylib.h>

#include "powerups.hpp"
#include "level.hpp"


void GlideAdd(Vector2 origin) {
    
    Level::Entity *glide = new Level::Entity();

    glide->tags = Level::IS_GLIDE;
    glide->origin = origin;
    glide->sprite = SPRITES->GlideItem;
    glide->hitbox = SpriteHitboxFromEdge(glide->sprite, glide->origin);

    LinkedList::Add(&Level::STATE->listHead, glide);

    TraceLog(LOG_TRACE, "Added glide item to level (x=%.1f, y=%.1f)",
                glide->hitbox.x, glide->hitbox.y);
}  

void GlideCheckAndAdd(Vector2 origin) {

    origin = SnapToGrid(origin, LEVEL_GRID);

    Rectangle hitbox = SpriteHitboxFromEdge(SPRITES->GlideItem, origin);
    if (Level::CheckCollisionWithAnything(hitbox)) return;

    if (!Level::GetGroundBeneathHitbox(hitbox)) {
        TraceLog(LOG_DEBUG, "Didn't add glide item to level, no ground beneath");
        return;
    }
    
    GlideAdd(origin);
}
