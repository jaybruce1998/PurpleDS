#include "LevelUpMove.h"
#include "Utils.h"
#include "debug.h"
#include "Gui.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

std::vector<LevelUpMove> LevelUpMove::LEVEL_UP_MOVES[153];

void LevelUpMove::buildLevelUpMoves() {
    const int count = 153;
    const size_t buffer_size = 1024; // Buffer for level-up move lines (larger due to move lists)
    
    // Open file directly (nitrofs should be auto-initialized by build system)
    FILE* file = fopen("nitro:/LevelUpMove.txt", "r");
    if (!file) {
        return; // Failed to open file
    }
    
    // Allocate buffer on heap (not stack)
    char* buffer = (char*)malloc(buffer_size);
    if (!buffer) {
        fclose(file);
        return; // Failed to allocate memory
    }
    
    // Read exactly 153 lines using optimized for loop
    for (int i = 0; i < count; i++) {
        if (fgets(buffer, buffer_size, file)) {
            // Remove newline character if present
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            
            // Process the line - split by comma and create level-up moves
            std::vector<std::string> a = utils::split(buffer, ',');
            LEVEL_UP_MOVES[i].clear();
            LEVEL_UP_MOVES[i].reserve(a.size());
            for (const std::string &s : a) {
                LEVEL_UP_MOVES[i].emplace_back(utils::trim(s));
            }
        } else {
            // Failed to read line, break early
            break;
        }
    }
    
    // Clean up
    free(buffer);
    fclose(file);
}

LevelUpMove::LevelUpMove(const std::string &s) {
    size_t i = s.find(' ');
    std::string l = s.substr(0, i);
    level = (l == "-" ? 1 : std::stoi(l));
    move = Move::getMove(s.substr(i + 1));
}

std::string LevelUpMove::toString() const {
    return std::to_string(level) + "=" + (move ? move->name : "");
}
