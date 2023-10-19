#include <raylib.h>

#include "camera.h"
#include "inlevel/player.h"
#include "global.h"

#define CAMERA_FOLLOW_LEFT_X SCREEN_WIDTH/4
#define CAMERA_FOLLOW_RIGHT_X (3*SCREEN_WIDTH)/5


MyCamera *CAMERA = 0;


void CameraInitialize() {
    CAMERA = MemAlloc(sizeof(MyCamera));

    CAMERA->pos = (Vector2) { 0, 0 };

    TraceLog(LOG_INFO, "Camera initialized.");
}

void CameraTick() {
    
    // TODO camera only ticks if player isn't moving
    // A way to do this might be to check player's velocity in a PLAYER_STATE global

    // TODO implement camera for other game modes
    if (STATE->mode != MODE_IN_LEVEL) {
        return;
    }

    if (LEVEL_PLAYER->hitbox.x < CAMERA->pos.x + CAMERA_FOLLOW_LEFT_X) {
        CAMERA->pos.x = LEVEL_PLAYER->hitbox.x - CAMERA_FOLLOW_LEFT_X;
    }
    else if (LEVEL_PLAYER->hitbox.x + LEVEL_PLAYER->hitbox.width > CAMERA->pos.x + CAMERA_FOLLOW_RIGHT_X) {
        CAMERA->pos.x = LEVEL_PLAYER->hitbox.x + LEVEL_PLAYER->hitbox.width - CAMERA_FOLLOW_RIGHT_X;
    }

    if (LEVEL_PLAYER->hitbox.y) {
        // TODO camera's vertical movement
    }
}
