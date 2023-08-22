#include <raylib.h>
#include <stdio.h>

#include "global.h"
#include "entity.h"
#include "player.h"
#include "enemy.h"
#include "level.h"

#define ENEMY_SPAWN_RATE 1.0f

typedef struct GameState {
    bool isPaused;
    bool isPlayerDead;

    PlayerMovementType playerMovementType;
} GameState;

void resetGameState(GameState *state, Entity **entities, Entity **player) {
    state->isPaused = false;
    state->isPlayerDead = false;

    state->playerMovementType = PLAYER_MOVEMENT_DEFAULT;

    DestroyAllEntities(*entities);
    *player = InitializePlayer(NULL);
    *entities = *player;

    InitializeLevel(*entities);

    SetEntityPosition(*player, SCREEN_WIDTH/4, (float)FLOOR_HEIGHT - (*player)->hitbox.height);
}

void updateWindowTitle() {
    char title[50];
    sprintf(title, "Jogo de Plataforma - %d FPS", GetFPS());
    SetWindowTitle(title);
}

int main(int argc, char **argv)
{
    GameState state;
    Entity *player;
    Entity *entities;

    double lastEnemySpawnTimestamp = -ENEMY_SPAWN_RATE;

    int gamepadIdx = 0;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Jogo de plataforma");
    SetTargetFPS(60);

    { // Initialization
        resetGameState(&state, &entities, &player);
    }

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        { // Game Update

            // Game State
            if (state.isPaused) {
                if (IsKeyPressed(KEY_ENTER)) {
                    if (state.isPlayerDead) {
                        resetGameState(&state, &entities, &player);
                    } else {
                        state.isPaused = false;
                    }
                }
                else goto render;
            }
            else if (IsKeyPressed(KEY_ENTER)) {
                state.isPaused = true;
            }


            if (IsKeyDown(KEY_BACKSPACE)) {
                int __dummy; __dummy++; // DEBUG: Put breakpoint here
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


            // Enemy
            if (GetTime() - lastEnemySpawnTimestamp > ENEMY_SPAWN_RATE) {
                InitializeEnemy(entities);
                lastEnemySpawnTimestamp = GetTime();
            }


            // Collision
            Entity *enemy = entities;
            do {

                if (enemy->components & IsEnemy) {

                    // Enemy offscreen
                    if  (enemy->hitbox.x + enemy->hitbox.width < 0) {
                        entities = DestroyEntity(enemy); // TODO: How does this break the loop?
                        break;
                    }

                    // Enemy hit player
                    if (CheckCollisionRecs(enemy->hitbox, playersUpperbody)) {
                        state.isPlayerDead = true;
                        state.isPaused = true;
                        break;
                    }

                    // Player hit enemy
                    if (CheckCollisionRecs(enemy->hitbox, playersLowebody)) {
                        entities = DestroyEntity(enemy); // TODO: How does this break the loop?
                        break;
                    }
                }

                enemy = enemy->next;
            } while (enemy != entities);


            TickAllEntities(entities, player);

            updateWindowTitle();
        }

render:
        { // Game Render
            BeginDrawing();

            ClearBackground(BLACK);

            DrawAllEntities(entities);

            // HUD
            if (state.isPaused && !state.isPlayerDead) DrawText("PAUSE", SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 30, RAYWHITE);
            if (state.isPlayerDead) DrawText("YOU DIED", SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 60, RAYWHITE);


            // Debug
            char entity_count[50];
            sprintf(entity_count, "%d entities", CountEntities(entities));
            DrawText(entity_count, 10, 20, 20, WHITE);


            // Gamepad debug
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