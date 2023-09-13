#include <raylib.h>
#include <stdio.h>

#include "global.h"
#include "entity.h"
#include "player.h"
#include "enemy.h"
#include "level.h"
#include "camera.h"

typedef struct GameState {
    bool isPaused;
    bool isPlayerDead;

    PlayerMovementType playerMovementType;
} GameState;

void resetGameState(GameState *state) {
    state->isPaused = false;
    state->isPlayerDead = false;

    state->playerMovementType = PLAYER_MOVEMENT_DEFAULT;

    DestroyAllEntities(ENTITIES);
    PLAYER = InitializePlayer(NULL);
    ENTITIES = PLAYER;
    CAMERA = InitializeCamera(ENTITIES);
    InitializeLevel(ENTITIES);
}

void updateWindowTitle() {
    char title[50];
    sprintf(title, "Jogo de Plataforma - %d FPS", GetFPS());
    SetWindowTitle(title);
}

int main(int argc, char **argv)
{
    GameState state;
    ENTITIES = 0;
    PLAYER = 0;
    CAMERA = 0;

    int gamepadIdx = 0;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Jogo de plataforma");
    SetTargetFPS(60);

    { // Initialization
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


            if (IsKeyDown(KEY_BACKSPACE)) {
                int __dummy; __dummy++; // DEBUG: Put breakpoint here
            }


            // Player
            Vector2 playerDelta = { 0.0f, 0.0f };

            if (IsKeyDown(KEY_Z)) state.playerMovementType = PLAYER_MOVEMENT_RUNNING;
            else state.playerMovementType = PLAYER_MOVEMENT_DEFAULT;

            if (IsKeyDown(KEY_RIGHT))
                MovePlayer(PLAYER, state.playerMovementType, PLAYER_MOVEMENT_RIGHT);
            if (IsKeyDown(KEY_LEFT) && PLAYER->hitbox.x > 0)
                MovePlayer(PLAYER, state.playerMovementType, PLAYER_MOVEMENT_LEFT);

            if (IsKeyPressed(KEY_X)) PlayerStartJump(PLAYER);

            // Editor
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                // TODO use a timer to not keep checking it every frame

                Vector2 mousePos = GetMousePosition();
                Vector2 blockPos = {
                    mousePos.x + CAMERA->hitbox.x,
                    mousePos.y + CAMERA->hitbox.y
                };
                AddBlockToLevel(ENTITIES, blockPos);
            }
            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {

                Vector2 mousePos = GetMousePosition();
                Vector2 enemyPos = {
                    mousePos.x + CAMERA->hitbox.x,
                    mousePos.y + CAMERA->hitbox.y
                };
                bool didPosition = AddEnemyToLevel(ENTITIES, enemyPos);
            }

            // Camera (debug)
            float cameraSpeed = 8.0f;
            if (IsKeyDown(KEY_A)) CAMERA->hitbox.x -= cameraSpeed;
            if (IsKeyDown(KEY_D)) CAMERA->hitbox.x += cameraSpeed;
            if (IsKeyDown(KEY_W)) CAMERA->hitbox.y -= cameraSpeed;
            if (IsKeyDown(KEY_S)) CAMERA->hitbox.y += cameraSpeed;




            {   // Collision

                if (PLAYER->hitbox.y > FLOOR_DEATH_HEIGHT) {
                    state.isPlayerDead = true;
                    state.isPaused = true;
                    goto render;
                }

                Entity *enemy = ENTITIES;
                do {

                    if (enemy->components & IsEnemy) {

                        // Enemy offscreen
                        if  (enemy->hitbox.x + enemy->hitbox.width < 0 ||
                                enemy->hitbox.y > FLOOR_DEATH_HEIGHT) {
                                    
                            ENTITIES = DestroyEntity(enemy); // TODO: How does this break the loop?
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
                            ENTITIES = DestroyEntity(enemy); // TODO: How does this break the loop?
                            break;
                        }
                    }

                    enemy = enemy->next;
                } while (enemy != ENTITIES);
            }


            TickAllEntities(ENTITIES, PLAYER);

            updateWindowTitle();
        }

render:
        { // Game Render
            BeginDrawing();

            ClearBackground(BLACK);


            // DrawAllEntities(entities);

            // Draw entities
            {
                Entity *currentItem = ENTITIES;

                do {
                    float inSceneX = currentItem->hitbox.x - CAMERA->hitbox.x;
                    float inSceneY = currentItem->hitbox.y - CAMERA->hitbox.y;

                    if (currentItem->components & IsPlayer ||
                        currentItem->components & IsEnemy)
                        if (currentItem->isFacingRight)
                            DrawTextureEx(currentItem->sprite, (Vector2){inSceneX, inSceneY}, 0, currentItem->spriteScale, WHITE);
                        else {
                            Rectangle source = (Rectangle){
                                0,
                                0,
                                -currentItem->sprite.width,
                                currentItem->sprite.height
                            };
                            Rectangle destination = (Rectangle){
                                inSceneX,
                                inSceneY,
                                currentItem->sprite.width * currentItem->spriteScale,
                                currentItem->sprite.height * currentItem->spriteScale
                            };
                            DrawTexturePro(currentItem->sprite, source, destination, (Vector2){ 0, 0 }, 0, WHITE);
                        }

                    else if (currentItem->components & IsLevelElement) {
                        
                        // Currently the only level element is a floor area to be tiled with a sprite

                        // How many tiles to be drawn in each axis
                        int xTilesCount = currentItem->hitbox.width / currentItem->spriteScale;
                        int yTilesCount = currentItem->hitbox.height / currentItem->spriteScale;

                        for (int xCurrent = 0; xCurrent < xTilesCount; xCurrent++) {
                            for (int yCurrent = 0; yCurrent < yTilesCount; yCurrent++) {
                                DrawTextureEx(
                                                currentItem->sprite,
                                                (Vector2){inSceneX + (xCurrent * currentItem->spriteScale),
                                                            inSceneY + (yCurrent * currentItem->spriteScale)},
                                                0,
                                                1,
                                                WHITE
                                            );
                            }
                        }
                    }

                    currentItem = currentItem->next;
                } while (currentItem != ENTITIES);
            }


            // HUD
            if (state.isPaused && !state.isPlayerDead) DrawText("PAUSE", SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 30, RAYWHITE);
            if (state.isPlayerDead) DrawText("YOU DIED", SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 60, RAYWHITE);


            // Debug
            char entity_count[50];
            sprintf(entity_count, "%d entities", CountEntities(ENTITIES));
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