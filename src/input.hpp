#pragma once


#include <string>
#include <functional>

#include "editor.hpp"


// The magnitude of the reading of an analog input to be read as a digital input
#define ANALOG_STICK_DIGITAL_THRESHOLD 0.5


typedef std::function<void(std::string)> TextInputCallback;

namespace Input {


class InputState {

public:

    // The text currently being inputed.
    std::string textInputed;

    // The callback to the text being currently inputed.
    TextInputCallback *textInputCallback;

    InputState() {}
};

extern InputState STATE;


void Initialize();

void Handle();

// Enters Text Input mode and, when the user finishes entering
// the texts, it passes it to the callback.
void GetTextInput(TextInputCallback *callback);

}
