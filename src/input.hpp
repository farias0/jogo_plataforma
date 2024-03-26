#pragma once


#include <string>
#include <functional>

#include "editor.hpp"


// The magnitude of the reading of an analog input to be read as a digital input
#define ANALOG_STICK_DIGITAL_THRESHOLD 0.5


typedef std::function<void(std::string)> TextInputCallback;

namespace Input {


typedef enum {
    GP_UP = 1,
    GP_RIGHT = 2,
    GP_DOWN = 3,
    GP_LEFT = 4,
    GP_Y = 5,
    GP_B = 6,
    GP_A = 7,
    GP_X = 8,
    GP_L1 = 9,
    GP_R1 = 11,
    GP_SELECT = 13,
    GP_START = 15,
    GP_L3 = 16,
    GP_R3 = 17,
} GamepadButton;

typedef enum {
    ANALOG_LEFT = 1,
    ANALOG_RIGHT = 2,
    ANALOG_UP = 4,
    ANALOG_DOWN = 8,
} AnalogStickDigitalDirection;

typedef enum {
    PLAYER_DIRECTION_STOP,
    PLAYER_DIRECTION_LEFT,
    PLAYER_DIRECTION_RIGHT
} PlayerMoveDirection;


class InputState {

public:

    // The text currently being inputed.
    std::string textInputed;

    // The callback to the text being currently inputed.
    TextInputCallback *textInputCallback;

    char leftStickCurrentState;
    char leftStickPreviousState;

    bool isHoldingRun;
    PlayerMoveDirection playerMoveDirection;

    InputState() {}
};


extern InputState STATE;


void Initialize();

void Handle();

// Enters Text Input mode and, when the user finishes entering
// the texts, it passes it to the callback.
void GetTextInput(TextInputCallback *callback);

}
