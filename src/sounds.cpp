#include <raylib.h>

#include "sounds.hpp"
#include "render.hpp"


namespace Sounds {


SoundsState STATE;


void Initialize() {

    STATE = SoundsState();

    TraceLog(LOG_INFO, "Sounds initialized.");
}

void Play(Sound sound) {

    if (STATE.isEnabled) PlaySound(sound);
}

void Toggle() {

    STATE.isEnabled = !STATE.isEnabled;

    if (STATE.isEnabled) {
        RenderPrintSysMessage("Som ligado");
        TraceLog(LOG_INFO, "Sound enabled.");
    }
    else {
        RenderPrintSysMessage("Som desligado");
        TraceLog(LOG_INFO, "Sound disabled.");
    }
}


}