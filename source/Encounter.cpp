#include "Encounter.h"
#include <iostream>
#include "PokeMap.h"
#include "Utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

void Encounter::buildEncounterRates() {
    const int count = 68;
    const size_t buffer_size = 2048; // Buffer for encounter data lines (larger due to complex data)
    
    // Open file directly (nitrofs should be auto-initialized by build system)
    FILE* file = fopen("nitro:/Encounter.txt", "r");
    if (!file) {
        return; // Failed to open file
    }
    
    // Allocate buffer on heap (not stack)
    char* buffer = (char*)malloc(buffer_size);
    if (!buffer) {
        fclose(file);
        return; // Failed to allocate memory
    }
    
    // Read exactly 68 lines using optimized for loop
    for (int i = 0; i < count; i++) {
        if (fgets(buffer, buffer_size, file)) {
            // Remove newline character if present
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            
            // Process encounter line - split by semicolon
            std::vector<std::string> a = utils::split(buffer, ';');
            if (a.size() > 0) {
                PokeMap::POKEMAPS[a[0]]->addEncounters(a);
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

Encounter::Encounter(const std::string &s) {
    std::vector<std::string> a = utils::split(s, ' ');
    chance = std::stoi(a[0]);
    rMonster = Monster::MONSTER_MAP[a[1]];
    rLevel = std::stoi(a[2]);
    bMonster = Monster::MONSTER_MAP[a[3]];
    bLevel = std::stoi(a[4]);
}

std::string Encounter::toString() const {
    return std::to_string(chance) + " " + (rMonster ? rMonster->name : "") + " " + std::to_string(rLevel) + " " +
           (bMonster ? bMonster->name : "") + " " + std::to_string(bLevel);
}
