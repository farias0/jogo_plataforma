#include "string"
#include "sstream"
#include "raylib.h"

#include "text_bank.hpp"
#include "files.hpp"

#define TEXT_BANK_FILEPATH "../assets/textbank.txt"
#define TEXT_FILE_DELIMITER " - "


namespace TextBank {

TextBankMap BANK;


void parseFileDataIntoBank(TextBankMap *bank, std::string fileData) {
    
    *bank = TextBankMap();
    std::istringstream stream(fileData);
    std::string delimiter(TEXT_FILE_DELIMITER);
    int currentId = -1;
    
    for (std::string line; std::getline(stream, line); ) {

        if (line.empty()) {
            // finish current entry
            currentId = -1; 
            continue;
        }

        size_t delimiterIdx = line.find(delimiter);
        std::string text;

        if (delimiterIdx == std::string::npos) {

            if (currentId == -1) {
                // outisde of an entry, with text, not starting a new entry
                goto invalid_line;
            }
            else {
                // append to existing entry
                if (line == "-") line = "";
                (*bank)[currentId] += "\n" + line;
                continue;
            }
        }

        // start a new entry

        try {
            currentId = std::stoi(line.substr(0, delimiterIdx));
        }
        catch (std::invalid_argument &e) {
            goto invalid_line;
        }

        text = line.substr(delimiterIdx + delimiter.length(), line.length());

        (*bank)[currentId] = text;

        continue;

invalid_line:
        TraceLog(LOG_FATAL, "Loaded file has invalid line: %s", line.c_str());
        exit(1);
    }

    TraceLog(LOG_DEBUG, "Text Bank loaded %d items.", bank->size());
}

void LoadFromDisk() {

    std::string text = Files::TextLoad(TEXT_BANK_FILEPATH);
    parseFileDataIntoBank(&BANK, text);
    TraceLog(LOG_INFO, "Text Bank initialized and loaded.");
}

}

