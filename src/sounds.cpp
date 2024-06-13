#include <raylib.h>

#include "sounds.hpp"
#include "render.hpp"


namespace Sounds {


SoundsState STATE;


void Initialize() {

    STATE = SoundsState();

    Toggle(); // disables by default

    TraceLog(LOG_INFO, "Sounds initialized.");
}

void Toggle() {

    if (IsEnabled())        SetMasterVolume(0);
    else                    SetMasterVolume(1);
}

bool IsEnabled() {
    return GetMasterVolume() != 0;
}

void Tick() {

    if (STATE.trackPlaying && !IsSoundPlaying(*STATE.trackPlaying)) // starts playing song and loops it when finished
        PlaySound(*STATE.trackPlaying);
}

void PlayEffect(Sound *sound) {
    PlaySound(*sound);
}

void PlayMusic(Sound *track) {
    STATE.trackPlaying = track;
}

void StopMusic() {
    if (STATE.trackPlaying) {
        StopSound(*STATE.trackPlaying);
        STATE.trackPlaying = 0;
    }
}

}
