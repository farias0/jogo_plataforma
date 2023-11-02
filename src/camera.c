#include <raylib.h>

#include "camera.h"
#include "level/level.h"
#include "core.h"

#define CAMERA_FOLLOW_LEFT_X SCREEN_WIDTH/4
#define CAMERA_FOLLOW_RIGHT_X (3*SCREEN_WIDTH)/5


MyCamera *CAMERA = 0;


static void tickOverworldCamera() {

    Dimensions dimensions = SpriteScaledDimensions(STATE->tileUnderCursor->sprite);

    CAMERA->pos.x = STATE->tileUnderCursor->gridPos.x
                    - (SCREEN_WIDTH/2) + (dimensions.width/2);
    CAMERA->pos.y = STATE->tileUnderCursor->gridPos.y
                    - (SCREEN_HEIGHT/2) + (dimensions.height/2);
}

static void tickLevelCamera() {

    if (!LEVEL_PLAYER) return;

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

void CameraInitialize() {
    CAMERA = MemAlloc(sizeof(MyCamera));

    CAMERA->pos = (Vector2) { 0, 0 };

    TraceLog(LOG_INFO, "Camera initialized.");
}

void CameraTick() {

    switch (STATE->mode)
    {
    case MODE_OVERWORLD:
        tickOverworldCamera();
        break;

    case MODE_IN_LEVEL:
        tickLevelCamera();
        break;
    }
}

Vector2 PosInScreenToScene(Vector2 pos) {
    return (Vector2){
        pos.x + CAMERA->pos.x,
        pos.y + CAMERA->pos.y
    };
}

Vector2 PosInSceneToScreen(Vector2 pos) {
    return (Vector2){
        pos.x - CAMERA->pos.x,
        pos.y - CAMERA->pos.y
    };
}
