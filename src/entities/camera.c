#include <raylib.h>

#include "camera.h"
#include "entity.h"
#include "../global.h"

#define CAMERA_FOLLOW_LEFT_X SCREEN_WIDTH/4
#define CAMERA_FOLLOW_RIGHT_X (3*SCREEN_WIDTH)/5


Entity *InitializeCamera(Entity *head, Entity **newCamera) {
    *newCamera = MemAlloc(sizeof(Entity));

    (*newCamera)->components = HasPosition +
                            IsCamera;
    (*newCamera)->hitbox = (Rectangle){ 0, 0, 0, 0 };

    return AddToEntityList(head, *newCamera);
}

void CameraTick() {
    
    // TODO camera only ticks if player isn't moving
    // A way to do this might be to check player's velocity in a PLAYER_STATE global

    // TODO implement camera for other game modes
    if (STATE->mode != InLevel) {
        return;
    }

    if (PLAYER->hitbox.x < CAMERA->hitbox.x + CAMERA_FOLLOW_LEFT_X) {
        CAMERA->hitbox.x = PLAYER->hitbox.x - CAMERA_FOLLOW_LEFT_X;
    }
    else if (PLAYER->hitbox.x + PLAYER->hitbox.width > CAMERA->hitbox.x + CAMERA_FOLLOW_RIGHT_X) {
        CAMERA->hitbox.x = PLAYER->hitbox.x + PLAYER->hitbox.width - CAMERA_FOLLOW_RIGHT_X;
    }

    if (PLAYER->hitbox.y) {
        // TODO camera's vertical movement
    }
}