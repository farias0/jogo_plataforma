#pragma once

#include "level.hpp"
#include "../animation.hpp"

#define COIN_ENTITY_ID          "coin"


class Coin : public Level::Entity, private Animation::IAnimated {

public:

    bool wasPickedUp;


    static Coin *AddFromPersistence();

    static Coin *Add(Vector2 origin);

    static void AddFromEditor(Vector2 origin, int interactionTags);

    void PickUp();

    bool IsDisabled() override;

    void Tick() override;

    void Draw() override;

    std::string PersistanceSerialize() override;
    
    void PersistenceParse(const std::string &data) override;

private:

    // How many frames its animation stays on the idle frame
    int idlePeriod;

    // How many frames its animation has been idle
    int timeIntoIdle;

    static Animation::Animation animationIdle;
    static Animation::Animation animationBlinking;

    void createAnimations();

    Animation::Animation *getCurrentAnimation();
};
