#pragma once

#include <raylib.h>
#include <map>

#include "../level.hpp"
#include "../../animation.hpp"


class INpc : public Level::Entity {

public:

    bool isFalling;


    static void Initialize();

    static void AddFromEditor(Vector2 pos, int interactionTags);

    virtual void Tick() override;

    virtual void Reset() override;

private:

    // Defines the different NPC types and which add function to use for each of them
    static std::map<std::string, void (*)(Vector2)> npcTypeMap;

};
