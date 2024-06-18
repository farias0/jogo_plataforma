#pragma once

#include "level.hpp"

#define COIN_ENTITY_ID          "coin"


class Coin : public Level::Entity {

public:

    bool wasPickedUp;


    static Coin *Add();

    static Coin *Add(Vector2 origin);

    static void AddFromEditor(Vector2 origin, int interactionTags);

    void PickUp();

    bool IsDisabled() override;

    void Draw() override;

    std::string PersistanceSerialize() override;
    
    void PersistenceParse(const std::string &data) override;
};
