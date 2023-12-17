#pragma once

#include <raylib.h>


namespace Sounds {


class SoundsState {

public:
    bool isEnabled;

    SoundsState() {
        isEnabled = false;
    }
};


extern SoundsState STATE;


void Initialize();

void Play(Sound sound);

// Toggles between sound on and off
void Toggle();


}
