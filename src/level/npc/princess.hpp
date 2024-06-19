#pragma once

#include "npc.hpp"


#define PRINCESS_ENTITY_ID          "princess"


class Princess : public INpc {

public:

    static Princess *AddFromPersistence();

    static Princess *Add(Vector2 pos);

    static void AddFromEditor(Vector2 pos);
};
