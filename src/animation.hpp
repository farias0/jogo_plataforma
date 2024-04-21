#pragma once

#include <vector>

#include "assets.hpp"


namespace Animation {


typedef struct {

    Sprite *sprite;
    int duration; // in frames
    
} Still;


class Animation {

public:
    std::vector<Still> stills;   

    /*
        Receives the frames that compose the animation, one at a time, in order of exhibition.
        
        @param sprite The sprite to be shown in this frame.
        @param duration For how many game frames this animation frame will be shown. 
    */
    void AddFrame(Sprite *sprite, int duration) {
        stills.push_back(Still{ sprite, duration });
    }

};


class IAnimated {

public:
 
    // Defines all Animations part of an animated entity's class.
    // Enforces that the entity has the animations it will use initialized,
    // ideally as attributes to its class (ex. Animation animationWalking).
    // TODO should be virtual so all instances of a class share the name animations
    // TODO try to make it private to the class
    // TODO call it as part of constructor
    virtual void createAnimations() = 0;

    // Uses the entity's internal state to decide which Animation
    // is right for the current frame;
    virtual Animation *getCurrentAnimation() = 0;

    // Returns the sprite for the current frame of the current animation.
    // To be called once every frame, as it moves the animation logic one step forwards.
    Sprite *animationTick() {

        Animation *oldAnimation = currentAnimation;
        currentAnimation = getCurrentAnimation();


        Still currentStill;

        if (oldAnimation == currentAnimation) {

            framesElapsedThisStill++;
            currentStill = currentAnimation->stills.at(currentStillIndex);

            if (framesElapsedThisStill > currentStill.duration) {

                // Next still
                currentStillIndex++;
                framesElapsedThisStill = 0;

                if (currentStillIndex > static_cast<int>(oldAnimation->stills.size()) - 1)
                    currentStillIndex = 0;

                currentStill = currentAnimation->stills.at(currentStillIndex);
            }

        }
        else {
            // Animation changed
            currentStillIndex = 0;
            framesElapsedThisStill = 0;
            currentStill = currentAnimation->stills.at(currentStillIndex);
        }

        return currentAnimation->stills.at(currentStillIndex).sprite;
    };

private:

    Animation *currentAnimation;
    int currentStillIndex;
    int framesElapsedThisStill;

};



} // namespace
