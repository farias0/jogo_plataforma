#include <raylib.h>

#include "camera.h"
#include "level/level.h"
#include "core.h"
#include "overworld.h"


#define CAMERA_FOLLOW_LEFT      SCREEN_WIDTH/4
#define CAMERA_FOLLOW_RIGHT     (3*SCREEN_WIDTH)/5
#define CAMERA_FOLLOW_UP        (1*SCREEN_HEIGHT)/4
#define CAMERA_FOLLOW_DOWN      (3*SCREEN_HEIGHT)/4


MyCamera *CAMERA = 0;


static bool isPanning = false;
static bool isPanned = false;
static Vector2 panningCameraOrigin;
static Vector2 panningCursorLastFrame;

static void followOverworldCamera() {

    Dimensions tileDimensions = SpriteScaledDimensions(STATE->tileUnderCursor->sprite);

    CAMERA->pos.x = STATE->tileUnderCursor->gridPos.x
                    - (SCREEN_WIDTH/2) + (tileDimensions.width/2);
    CAMERA->pos.y = STATE->tileUnderCursor->gridPos.y
                    - (SCREEN_HEIGHT/2) + (tileDimensions.height/2);
}

static void followLevelCamera() {

    if (!LEVEL_PLAYER) {
        TraceLog(LOG_WARNING, "Camera can't follow player, no reference to them.");
        return;
    }

    if (LEVEL_PLAYER->hitbox.x < CAMERA->pos.x + CAMERA_FOLLOW_LEFT) {
        CAMERA->pos.x = LEVEL_PLAYER->hitbox.x - CAMERA_FOLLOW_LEFT;
    }
    else if (LEVEL_PLAYER->hitbox.x + LEVEL_PLAYER->hitbox.width > CAMERA->pos.x + CAMERA_FOLLOW_RIGHT) {
        CAMERA->pos.x = LEVEL_PLAYER->hitbox.x + LEVEL_PLAYER->hitbox.width - CAMERA_FOLLOW_RIGHT;
    }
    if (LEVEL_PLAYER->hitbox.y < CAMERA->pos.y + CAMERA_FOLLOW_UP) {
        CAMERA->pos.y = LEVEL_PLAYER->hitbox.y - CAMERA_FOLLOW_UP;
    }
    else if (LEVEL_PLAYER->hitbox.y + LEVEL_PLAYER->hitbox.height > CAMERA->pos.y + CAMERA_FOLLOW_DOWN) {
        CAMERA->pos.y = LEVEL_PLAYER->hitbox.y + LEVEL_PLAYER->hitbox.height - CAMERA_FOLLOW_DOWN;
    }
}


static void tickOverworldCamera() {

    followOverworldCamera();
}

static void tickLevelCamera() {

    if (!LEVEL_PLAYER) return;
    followLevelCamera();
}

void CameraInitialize() {
    CAMERA = MemAlloc(sizeof(MyCamera));

    CAMERA->pos = (Vector2) { 0, 0 };

    TraceLog(LOG_INFO, "Camera initialized.");
}

void CameraTick() {

    if (isPanned) return;
    if (STATE->isEditorEnabled) return;

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

void CameraFollow() {

    switch (STATE->mode)
    {
    case MODE_OVERWORLD:
        followOverworldCamera();
        break;

    case MODE_IN_LEVEL:
        followLevelCamera();
        break;
    }
}

void CameraLevelCentralizeOnPlayer() {

    if (!LEVEL_PLAYER) {
        TraceLog(LOG_ERROR, "Camera can't centralize on Player because Player instance couldn't be found.");
        return;
    }

    CAMERA->pos.x = LEVEL_PLAYER->hitbox.x - SCREEN_WIDTH/3;
    CAMERA->pos.y = LEVEL_PLAYER->hitbox.y - (3*SCREEN_HEIGHT)/5;

    CameraPanningReset();
    TraceLog(LOG_TRACE, "Camera centralized on Player.");
}

void CameraPanningMove(Vector2 mousePos) {

    if (!isPanning) {

        isPanning = true;
        panningCursorLastFrame = mousePos;

        if (!isPanned) panningCameraOrigin = CAMERA->pos;

        TraceLog(LOG_TRACE, "Camera started panning.");
        return;
    }

    CAMERA->pos.x -= mousePos.x - panningCursorLastFrame.x;
    CAMERA->pos.y -= mousePos.y - panningCursorLastFrame.y;

    panningCursorLastFrame = mousePos;
    isPanned = true;
}

void CameraPanningStop() {

    if (!isPanning) TraceLog(LOG_ERROR, "Camera is asked to stop panning, but panning flag is false.");

    isPanning = false;
    TraceLog(LOG_TRACE, "Camera stopped panning.");
}

void CameraPanningReset() {
    
    if (!isPanned) return;
    
    CAMERA->pos = panningCameraOrigin;
    isPanned = false;
    TraceLog(LOG_TRACE, "Camera panning reset.");
}

bool CameraIsPanned() {
    return isPanned;
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
