#pragma once

#include <raylib.h>

#include "../level.hpp"
#include "../../animation.hpp"


class INpc : public Level::Entity {

public:

    bool isFalling;


    static void CheckAndAdd(Vector2 pos, int interactionTags);

    virtual void Tick() override;

    virtual void Reset() override;

    void PersistenceParse(const std::string &data) override;
    std::string PersistanceSerialize() override;

    virtual std::string PersistanceEntityID() = 0;
};
