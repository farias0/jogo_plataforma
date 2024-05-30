#pragma once

#include <raylib.h>


namespace Sounds {


class SoundsState {

public:

    // Music track
    Sound *trackPlaying;
};


extern SoundsState STATE;


void Initialize();

// Toggles between sound on and off
void Toggle();

bool IsEnabled();

void Tick();

// Plays an effect clip once
void PlayEffect(Sound *clip);

// Plays a song continuously 
void PlayMusic(Sound *track);

// Stops the song being played
void StopMusic();

}
