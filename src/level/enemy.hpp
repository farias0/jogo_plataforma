#ifndef _ENEMY_H_INCLUDED_
#define _ENEMY_H_INCLUDED_


#include <raylib.h>

#include "level.hpp"


class Enemy : public Level::Entity {

public:

    // Initializes and adds an enemy to the level
    static Enemy *Add();

    // Initializes and adds an enemy to the level in the given origin
    static Enemy *Add(Vector2 origin);

    // Initializes and adds an enemy to the level in the given origin,
    // if there are not other elements there already
    static void CheckAndAdd(Vector2 origin);

    // Kills a given enemy
    void Kill();

    // Runs the update routine of a given enemy
    void Tick();

    void Draw();

    std::string PersistanceEntityID() {
        static const std::string persistanceEntityID = "enemy";
        return persistanceEntityID;
    }
};


#endif // _ENEMY_H_INCLUDED_
