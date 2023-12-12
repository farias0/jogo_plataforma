#pragma once


#include <string>
#include <functional>

#include "editor.hpp"


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
