#include <raylib.h>
#include <stdio.h>

#include "global.h"
#include "entity.h"
#include "player.h"

typedef struct GameState {
    bool isPaused;
    bool isPlayerDead;

    PlayerMovementType playerMovementType;
} GameState;

void resetGameState(GameState *state, Entity *player) {
    state->isPaused = false;
    state->isPlayerDead = false;

    state->playerMovementType = PLAYER_MOVEMENT_DEFAULT;

    SetEntityPosition(player, SCREEN_WIDTH/4, (float)FLOOR_HEIGHT-player->hitbox.height);
}

void updateWindowTitle() {
    char title[50];
    sprintf(title, "Jogo de Plataforma - %d FPS", GetFPS());
    SetWindowTitle(title);
}

int main(int argc, char **argv)
{
    GameState state;
    Entity *player = InitializePlayer(NULL);
    Entity *entities = player;

    int gamepadIdx = 0;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Jogo de plataforma");
    SetTargetFPS(60);

    resetGameState(&state, player);

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        { // Game Update

            // Game State
            if (state.isPaused) {
                if (IsKeyPressed(KEY_ENTER)) {
                    if (state.isPlayerDead) {
                        resetGameState(&state, player);
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

            if (IsKeyDown(KEY_RIGHT) && (player->hitbox.x + player->hitbox.width) < SCREEN_WIDTH)
                MovePlayer(player, state.playerMovementType, PLAYER_MOVEMENT_RIGHT);
            if (IsKeyDown(KEY_LEFT) && player->hitbox.x > 0)
                MovePlayer(player, state.playerMovementType, PLAYER_MOVEMENT_LEFT);

            if (IsKeyPressed(KEY_X)) PlayerStartJump(player);

            TickAllEntities(entities);

            updateWindowTitle();
        }

render:
        { // Game Render
            BeginDrawing();

            ClearBackground(BLACK);

            DrawAllEntities(entities);

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