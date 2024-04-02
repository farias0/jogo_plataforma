#pragma once

#include "level.hpp"


class GrapplingHook : public Level::Entity {

public:
    Vector2 start;
    Vector2 end;

    float currentLength;

    static GrapplingHook *Initialize();
    void Tick();
    void Draw();

    ~GrapplingHook();
};
