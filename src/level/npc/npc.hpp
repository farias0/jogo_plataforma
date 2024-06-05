#pragma once

#include <raylib.h>

#include "../level.hpp"
#include "../../animation.hpp"


class INpc : public Level::Entity {

public:

    bool isFalling;


    static void AddFromEditor(Vector2 pos, int interactionTags); // TODO NPCs should be managed by the level module

    virtual void Tick() override;

    virtual void Reset() override;

    void PersistenceParse(const std::string &data) override;
    std::string PersistanceSerialize() override;

    virtual std::string PersistanceEntityID() = 0;
};
