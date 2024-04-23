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

    // Where the entity sets up its animations (which should be static to the entity).
    virtual void createAnimations() = 0;

    // Where the entity decides which Animation is right for the current game frame,
    // acording to its internal state.
    virtual Animation *getCurrentAnimation() = 0;

    // To be called during the entity initialization.
    // Creating animations during the entity's intiialization guarantees that the sprites
    // are already initialized.
    void initializeAnimationSystem() {
        if (!isInitialized) {
            createAnimations();
        }
    }

    // Returns the sprite for the current frame of the current animation.
    // To be called once every frame. It moves the animation logic one step forwards.
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

    // Controls if the static animations for this entity type were already created
    static bool isInitialized;

    // Animation to be used in the current frame
    Animation *currentAnimation;
    // In which Still currentAnimation is
    int currentStillIndex;
    // For how long the entity has been in this still
    int framesElapsedThisStill;

};



} // namespace
