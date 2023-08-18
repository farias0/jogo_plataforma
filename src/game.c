#include <raylib.h>
#include <stdio.h>

#include "global.h"
#include "player.h"

typedef struct GameState {
    bool isPaused;
    bool isPlayerDead;

    PlayerMovementType playerMovementType;
} GameState;

void resetGameState(GameState *state) {
    state->isPaused = false;
    state->isPlayerDead = false;

    state->playerMovementType = PLAYER_MOVEMENT_DEFAULT;

    SetPlayerPosition((Vector2){ (float)SCREEN_WIDTH/4, (float)FLOOR_HEIGHT-playerHitbox.height });
}

int main(int argc, char **argv)
{
    GameState state;

    int gamepadIdx = 0;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Jogo de plataforma");
    SetTargetFPS(60);

    { // Initialization
        InitializePlayer();

        resetGameState(&state);
    }

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        { // Game Update

            // Game State
            if (state.isPaused) {
                if (IsKeyPressed(KEY_ENTER)) {
                    if (state.isPlayerDead) {
                        resetGameState(&state);
                    } else {
                        state.isPaused = false;
                    }
                }
                else goto render;
            }
            else if (IsKeyPressed(KEY_ENTER)) {
                state.isPaused = true;
            }

            // Player
            Vector2 playerDelta = { 0.0f, 0.0f };

            if (IsKeyDown(KEY_Z)) state.playerMovementType = PLAYER_MOVEMENT_RUNNING;
            else state.playerMovementType = PLAYER_MOVEMENT_DEFAULT;

            if (IsKeyDown(KEY_RIGHT) && (playerHitbox.x + playerHitbox.width) < SCREEN_WIDTH)
                MovePlayer(state.playerMovementType, PLAYER_MOVEMENT_RIGHT);
            if (IsKeyDown(KEY_LEFT) && playerHitbox.x > 0)
                MovePlayer(state.playerMovementType, PLAYER_MOVEMENT_LEFT);
        }

render:
        { // Game Render
            BeginDrawing();

            ClearBackground(BLACK);

            DrawPlayer();

            if (state.isPaused && !state.isPlayerDead) DrawText("PAUSE", SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 30, RAYWHITE);
            if (state.isPlayerDead) DrawText("YOU DIED", SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 60, RAYWHITE);


            DrawRectangle(0, FLOOR_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT, PURPLE);


            // Gamepad
            if (IsGamepadAvailable(gamepadIdx)) {
                char gpad[27];
                sprintf(gpad, "Gamepad index: %i", gamepadIdx);
                DrawText(gpad, 10, 900, 20, WHITE);
                DrawText(GetGamepadName(gamepadIdx), 10, 920, 20, WHITE);
            }

            EndDrawing();
        }
    }

    CloseWindow();
    return 0;
}