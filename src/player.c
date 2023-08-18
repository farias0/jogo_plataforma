#include <raylib.h>
#include "player.h"

#define PLAYER_SPRITE_SCALE 2

// sprite is 32x64 pixels
#define PLAYER_WIDTH (float)(PLAYER_SPRITE_SCALE * 32)
#define PLAYER_HEIGHT (float)(PLAYER_SPRITE_SCALE * 64)

#define PLAYER_SPEED_DEFAULT 4.0f
#define PLAYER_SPEED_FAST 8.0f

static Texture2D sprite;

Rectangle playerHitbox = { 0.0f, 0.0f, PLAYER_WIDTH, PLAYER_HEIGHT };

void InitializePlayer() {
    sprite = LoadTexture("../assets/player_default_1.png");
}

void SetPlayerPosition(Vector2 pos) {
    playerHitbox.x = pos.x;
    playerHitbox.y = pos.y;
}

void MovePlayer(PlayerMovementType type, PlayerMovementDirection direction) {
    float amount = PLAYER_SPEED_DEFAULT;
    if (type == PLAYER_MOVEMENT_RUNNING) amount = PLAYER_SPEED_FAST;

    switch (direction) {
        case PLAYER_MOVEMENT_LEFT: playerHitbox.x -= amount; break;
        case PLAYER_MOVEMENT_RIGHT: playerHitbox.x += amount; break;
    }
}

void DrawPlayer() {
    DrawTextureEx(sprite, (Vector2){playerHitbox.x, playerHitbox.y}, 0, PLAYER_SPRITE_SCALE, WHITE);
}