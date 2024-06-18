#pragma once

#include "level.hpp"
#include "../animation.hpp"

#define COIN_ENTITY_ID          "coin"


class Coin : public Level::Entity, private Animation::IAnimated {

public:

    bool wasPickedUp;


    static Coin *Add();

    static Coin *Add(Vector2 origin);

    static void AddFromEditor(Vector2 origin, int interactionTags);

    void PickUp();

    bool IsDisabled() override;

    void Tick() override;

    void Draw() override;

    std::string PersistanceSerialize() override;
    
    void PersistenceParse(const std::string &data) override;

private:

    static Animation::Animation animationDefault;

    void createAnimations();

    Animation::Animation *getCurrentAnimation();
};
