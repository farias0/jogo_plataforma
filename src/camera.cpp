#include <raylib.h>

#include "camera.hpp"
#include "level/player.hpp"
#include "core.hpp"
#include "overworld.hpp"
#include "editor.hpp"


#define CAMERA_FOLLOW_LEFT      SCREEN_WIDTH/4
#define CAMERA_FOLLOW_RIGHT     (3*SCREEN_WIDTH)/5
#define CAMERA_FOLLOW_UP        (1*SCREEN_HEIGHT)/4
#define CAMERA_FOLLOW_DOWN      (3*SCREEN_HEIGHT)/4

#define ZOOM_STEP               0.10
#define ZOOM_MIN                0.20
#define ZOOM_MAX                2


MyCamera *CAMERA = 0;


static bool isPanning = false;
static bool isPanned = false;
static Vector2 panningCameraOrigin;
static Vector2 panningCursorLastFrame;


static void followOverworldCamera() {

    Dimensions tileDimensions = SpriteScaledDimensions(OW_STATE->tileUnderCursor->sprite);

    CAMERA->pos.x = OW_STATE->tileUnderCursor->gridPos.x
                    - (SCREEN_WIDTH/2) + (tileDimensions.width/2);
    CAMERA->pos.y = OW_STATE->tileUnderCursor->gridPos.y
                    - (SCREEN_HEIGHT/2) + (tileDimensions.height/2);
}

static void followLevelCamera() {

    if (!PLAYER) {
        TraceLog(LOG_WARNING, "Camera can't follow player, no reference to them.");
        return;
    }

    if (PLAYER->hitbox.x < CAMERA->pos.x + CAMERA_FOLLOW_LEFT) {
        CAMERA->pos.x = PLAYER->hitbox.x - CAMERA_FOLLOW_LEFT;
    }
    else if (PLAYER->hitbox.x + PLAYER->hitbox.width > CAMERA->pos.x + CAMERA_FOLLOW_RIGHT) {
        CAMERA->pos.x = PLAYER->hitbox.x + PLAYER->hitbox.width - CAMERA_FOLLOW_RIGHT;
    }
    if (PLAYER->hitbox.y < CAMERA->pos.y + CAMERA_FOLLOW_UP) {
        CAMERA->pos.y = PLAYER->hitbox.y - CAMERA_FOLLOW_UP;
    }
    else if (PLAYER->hitbox.y + PLAYER->hitbox.height > CAMERA->pos.y + CAMERA_FOLLOW_DOWN) {
        CAMERA->pos.y = PLAYER->hitbox.y + PLAYER->hitbox.height - CAMERA_FOLLOW_DOWN;
    }
}


static void tickOverworldCamera() {

    followOverworldCamera();
}

static void tickLevelCamera() {

    if (!PLAYER) return;
    followLevelCamera();
}

void CameraInitialize() {
    CAMERA = (MyCamera *) MemAlloc(sizeof(MyCamera));

    CAMERA->pos = { 0, 0 };
    CAMERA->zoom = 1;

    TraceLog(LOG_INFO, "Camera initialized.");
}

void CameraTick() {

    if (isPanned) return;
    if (EDITOR_STATE->isEnabled) return;

    switch (GAME_STATE->mode)
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

    switch (GAME_STATE->mode)
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

    if (!PLAYER) {
        TraceLog(LOG_ERROR, "Camera can't centralize on Player because Player instance couldn't be found.");
        return;
    }

    CAMERA->pos.x = PLAYER->hitbox.x - SCREEN_WIDTH/3;
    CAMERA->pos.y = PLAYER->hitbox.y - SCREEN_HEIGHT/2;

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

    CAMERA->pos.x -= (mousePos.x - panningCursorLastFrame.x) / CAMERA->zoom;
    CAMERA->pos.y -= (mousePos.y - panningCursorLastFrame.y) / CAMERA->zoom;

    panningCursorLastFrame = mousePos;
    isPanned = true;
}

void CameraPanningStop() {

    if (!isPanning) TraceLog(LOG_ERROR, "Camera is asked to stop panning, but panning flag is false.");

    isPanning = false;
    TraceLog(LOG_TRACE, "Camera stopped panning.");
}

void CameraPanningReset() {
    
    CAMERA->pos = panningCameraOrigin;
    CAMERA->zoom = 1;
    isPanned = false;
    TraceLog(LOG_TRACE, "Camera panning reset.");
}

bool CameraIsPanned() {
    return isPanned;
}

void CameraZoomIn() {
    CAMERA->zoom += ZOOM_STEP;
    if (CAMERA->zoom >= ZOOM_MAX) CAMERA->zoom = ZOOM_MAX;
}

void CameraZoomOut() {
    CAMERA->zoom -= ZOOM_STEP;
    if (CAMERA->zoom <= ZOOM_MIN) CAMERA->zoom = ZOOM_MIN;
}

/*
    Zooming works by keeping the 0,0 pinned and then streching around
    -- which means it streches towards the botttom right of the screen.
*/

Vector2 PosInScreenToScene(Vector2 pos) {
    return {
        pos.x / CAMERA->zoom + CAMERA->pos.x,
        pos.y / CAMERA->zoom + CAMERA->pos.y
    };
}

Vector2 PosInSceneToScreen(Vector2 pos) {
    return {
        (pos.x - CAMERA->pos.x) * CAMERA->zoom,
        (pos.y - CAMERA->pos.y) * CAMERA->zoom
    };
}

float ScaleInSceneToScreen(float value) {
    return value * CAMERA->zoom;
}

Dimensions DimensionsInSceneToScreen(Dimensions dim) {
    return {
        dim.width * CAMERA->zoom,
        dim.height * CAMERA->zoom
    };
}
