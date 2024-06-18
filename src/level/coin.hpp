#pragma once

#include "level.hpp"

#define COIN_ENTITY_ID          "coin"


class Coin : public Level::Entity {

public:

    static Coin *Add();

    static Coin *Add(Vector2 origin);

    static void AddFromEditor(Vector2 origin, int interactionTags);

    std::string PersistanceSerialize() override;
    
    void PersistenceParse(const std::string &data) override;
};
