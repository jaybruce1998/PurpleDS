#include "Evolution.h"
#include "Utils.h"
#include "debug.h"
#include "Gui.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

Evolution::Evolution(const std::string &stone, int evo) {
    this->stone = stone;
    this->evo = evo;
}

Evolution::Evolution(int level, int evo) {
    this->level = level;
    this->evo = evo;
}

Evolution* Evolution::EVOLUTIONS[153];

void Evolution::buildEvolutions() {
    const int count = 65;
    const size_t buffer_size = 256; // Buffer for evolution data lines
    
    // Open file directly (nitrofs should be auto-initialized by build system)
    FILE* file = fopen("nitro:/Evolution.txt", "r");
    if (!file) {
        return; // Failed to open file
    }
    
    // Allocate buffer on heap (not stack)
    char* buffer = (char*)malloc(buffer_size);
    if (!buffer) {
        fclose(file);
        return; // Failed to allocate memory
    }
    
    // Read exactly 65 lines using optimized for loop
    for (int i = 0; i < count; i++) {
        if (fgets(buffer, buffer_size, file)) {
            // Remove newline character if present
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            
            // Process evolution line - split by comma
            std::vector<std::string> a = utils::split(buffer, ',');
            int index = std::stoi(a[0]);
            int evo = std::stoi(a[2]);
            
            if (!a[1].empty() && a[1][0] == '(') {
                std::string levelPart = a[1];
                size_t close = levelPart.find(')');
                std::string num = levelPart.substr(levelPart.find(' ') + 1, close - levelPart.find(' ') - 1);
                EVOLUTIONS[index] = new Evolution(std::stoi(num), evo);
            } else {
                EVOLUTIONS[index] = new Evolution(a[1], evo);
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

std::string Evolution::toString() const {
    if (stone.empty()) {
        return "Level " + std::to_string(level) + "->" + std::to_string(evo);
    }
    return stone + "->" + std::to_string(evo);
}
