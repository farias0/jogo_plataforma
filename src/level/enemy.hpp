#ifndef _ENEMY_H_INCLUDED_
#define _ENEMY_H_INCLUDED_


#include <raylib.h>

#include "level.hpp"
#include "../animation.hpp"

#define ENEMY_PERSISTENCE_ID        "enemy"
#define ENEMY_DUMMY_PERSISTENCE_ID  "enemy_dummy_spike"


class Enemy : public Level::Entity {

public:

    // Initializes and adds an enemy to the level
    static Enemy *Add();

    // Initializes and adds an enemy to the level in the given origin
    static Enemy *Add(Vector2 origin);

    // Initializes and adds an enemy to the level in the given origin,
    // if there are not other elements there already
    static void CheckAndAdd(Vector2 origin, int interactionTags);

    // Kills a given enemy
    virtual void Kill();

    // Runs the update routine of a given enemy
    void Tick();

    virtual void Draw();

    std::string PersistanceEntityID() {
        return ENEMY_PERSISTENCE_ID;
    }
};


class EnemyDummySpike : public Enemy, public Animation::IAnimated {

public:

    static EnemyDummySpike *Add();

    static EnemyDummySpike *Add(Vector2 origin);

    static void CheckAndAdd(Vector2 origin, int interactionTags);

    void Reset() override;

    void Kill() override;

    void Tick() override;

    void Draw() override;

    std::string PersistanceEntityID() override {
        return ENEMY_DUMMY_PERSISTENCE_ID;
    }

private:

    static Animation::Animation animationDefault;
    static Animation::Animation animationPoppingOut;
    static Animation::Animation animationPopppedOut;

    unsigned int popOutAnimationCountdown;


    void setToSpike();
    void setToEnemy();

    void createAnimations();

    Animation::Animation *getCurrentAnimation();
};


#endif // _ENEMY_H_INCLUDED_
