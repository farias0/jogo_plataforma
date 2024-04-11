#pragma once

#include "level.hpp"


class GrapplingHook : public Level::Entity {

public:
    Vector2 start;
    Vector2 end;

    float currentLength;
    Level::Entity *attachedTo;

    // Used by the swinging simulation
    double currentAngle; // In radians
    double angularVelocity;

    static GrapplingHook *Initialize();
    
    void Tick();

    // Simulates swing movement 
    void Swing();

    // Sets this this hook's start position to where the Player's hands are
    void FollowPlayer();

    void Draw();

    ~GrapplingHook();
};
