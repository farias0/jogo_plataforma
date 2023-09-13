#include <raylib.h>

#include "camera.h"
#include "entity.h"

Entity *InitializeCamera(Entity *listItem) {
    Entity *newCamera = MemAlloc(sizeof(Entity));

    newCamera->components = HasPosition;
    newCamera->hitbox = (Rectangle){ 0, 0, 0, 0 };

    AddToEntityList(listItem, newCamera);

    return newCamera;
}