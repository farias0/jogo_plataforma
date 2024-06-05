#pragma once

#include "npc.hpp"


#define PRINCESS_PERSISTENCE_ID          "princess"


class Princess : public INpc {

public:

    static Princess *Add();

    static Princess *Add(Vector2 pos);

    static void CheckAndAdd(Vector2 pos, int interactionTags);

    std::string PersistanceEntityID() {
        return PRINCESS_PERSISTENCE_ID;
    }
};