#include <raylib.h>
#include <stdio.h>

#include "global.h"
#include "entity.h"
#include "player.h"
#include "enemy.h"
#include "level.h"
#include "camera.h"
#include "render.h"


void updateWindowTitle() {
    char title[50];
    sprintf(title, "Jogo de Plataforma - %d FPS", GetFPS());
    SetWindowTitle(title);
}

int main(int argc, char **argv)
{
    STATE = MemAlloc(sizeof(GameState));
    ENTITIES = 0;
    PLAYER = 0;
    CAMERA = 0;

    // debug
    STATE->gamepadIdx = 0;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Jogo de plataforma");
    SetTargetFPS(60);

    { // Initialization
        ResetGameState();
    }

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        { // Game Update

            // Game State
            if (STATE->isPaused) {
                if (IsKeyPressed(KEY_ENTER)) {
                    if (STATE->isPlayerDead) {
                        ResetGameState();
                    } else {
                        STATE->isPaused = false;
                    }
                }
                else goto render;
            }
            else if (IsKeyPressed(KEY_ENTER)) {
                STATE->isPaused = true;
            }


            if (IsKeyDown(KEY_BACKSPACE)) {
                int __dummy; __dummy++; // DEBUG: Put breakpoint here
            }


            // Player
            Vector2 playerDelta = { 0.0f, 0.0f };

            if (IsKeyDown(KEY_Z)) STATE->playerMovementType = PLAYER_MOVEMENT_RUNNING;
            else STATE->playerMovementType = PLAYER_MOVEMENT_DEFAULT;

            if (IsKeyDown(KEY_RIGHT))
                MovePlayer(PLAYER, STATE->playerMovementType, PLAYER_MOVEMENT_RIGHT);
            if (IsKeyDown(KEY_LEFT) && PLAYER->hitbox.x > 0)
                MovePlayer(PLAYER, STATE->playerMovementType, PLAYER_MOVEMENT_LEFT);

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
                    STATE->isPlayerDead = true;
                    STATE->isPaused = true;
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
                            STATE->isPlayerDead = true;
                            STATE->isPaused = true;
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

            RenderAllEntities();

            RenderHUD();

            EndDrawing();
        }
    }

    CloseWindow();
    return 0;
}