#pragma once

#include "unordered_map"
#include "string"

namespace TextBank {

/*
    Reads the text that will go into the textboxes from a file
    and save it to a hash map, to be used by the game.    
*/

typedef std::unordered_map<int, std::string> TextBankMap;

extern TextBankMap BANK;

void InitializeAndLoad();

}
