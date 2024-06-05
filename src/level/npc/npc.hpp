#pragma once

#include <raylib.h>
#include <map>

#include "../level.hpp"
#include "../../animation.hpp"


class INpc : public Level::Entity {

public:

    bool isFalling;


    static void Initialize();

    static void CheckAndAdd(Vector2 pos, int interactionTags);

    virtual void Tick() override;

    virtual void Reset() override;

    void PersistenceParse(const std::string &data) override;
    std::string PersistanceSerialize() override;

    virtual std::string PersistanceEntityID() = 0;


private:

    // Defines the different NPC types and which add function to use for each of them
    static std::map<std::string, void (*)(Vector2)> npcTypeMap;

};
