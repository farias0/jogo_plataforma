#include <raylib.h>

#include "powerups.h"
#include "level.h"


void GlideAdd(Vector2 origin) {
    
    LevelEntity *glide = MemAlloc(sizeof(LevelEntity));

    glide->components = LEVEL_IS_GLIDE;
    glide->origin = origin;
    glide->sprite = GlideItemSprite;
    glide->hitbox = SpriteHitboxFromEdge(glide->sprite, glide->origin);

    LinkedListAdd(&LEVEL_LIST_HEAD, glide);

    TraceLog(LOG_TRACE, "Added glide item to level (x=%.1f, y=%.1f)",
                glide->hitbox.x, glide->hitbox.y);
}  

void GlideCheckAndAdd(Vector2 origin) {

    origin = SnapToGrid(origin, LEVEL_GRID);

    Rectangle hitbox = SpriteHitboxFromEdge(GlideItemSprite, origin);
    if (LevelCheckCollisionWithAnything(hitbox)) return;

    if (!LevelGetGroundBeneathHitbox(hitbox)) {
        TraceLog(LOG_DEBUG, "Didn't add glide item to level, no ground beneath");
        return;
    }
    
    GlideAdd(origin);
}
