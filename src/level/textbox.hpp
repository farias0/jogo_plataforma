#pragma once


#include <string>

#include "level.hpp"


class Textbox : public Level::Entity {

public:

    int textId;


    static Textbox *Add();

    // Initializes and adds a textbox button to the level
    static Textbox *Add(Vector2 pos, int textId);

    // Initializes and adds a a textbox button to the level in the
    // given origin, if there are no other entities there already
    static void CheckAndAdd(Vector2 pos);

    void PersistenceParse(const std::string &data) override;
    std::string PersistanceSerialize() override;

private:
    static void createFromIdInput(Vector2 pos, std::string input);

};
