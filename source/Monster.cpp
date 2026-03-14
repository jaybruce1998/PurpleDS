#include "Monster.h"
#include "Utils.h"
#include "Types.h"
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include "debug.h"

Monster Monster::MONSTERS[153];
std::unordered_map<std::string, Monster*> Monster::MONSTER_MAP;

Monster::Monster() {
    name = "";
}

Monster::Monster(const std::string &s, int dexNum) {
    std::vector<std::string> a = utils::split(s, ',');
    name = a[0];
    
    // Parse types and pack into single byte
    std::vector<std::string> typeStrs = utils::split(a[1], '/');
    u8 type1 = Types::getTypeIndex(typeStrs[0]);
    u8 type2 = (typeStrs.size() > 1) ? Types::getTypeIndex(typeStrs[1]) : 15; // 15 = no type
    setTypes(type1, type2);
    
    hp = std::stoi(a[2]);
    atk = std::stoi(a[3]);
    def = std::stoi(a[4]);
    spatk = std::stoi(a[5]);
    spdef = std::stoi(a[6]);
    spd = std::stoi(a[7]);
    this->dexNum = dexNum;
}

void Monster::buildMonsters() {
    const u8 count = 153;
    const size_t buffer_size = 256; // Buffer for monster data lines
    
    // Open file directly (nitrofs should be auto-initialized by build system)
    FILE* file = fopen("nitro:/Monster.txt", "r");
    if (!file) {
        debugCheckpoint("No monsters");
        return; // Failed to open file
    }
    
    // Allocate buffer on heap (not stack)
    char* buffer = (char*)malloc(buffer_size);
    if (!buffer) {
        fclose(file);
        return; // Failed to allocate memory
    }
    
    // Read exactly 153 lines using optimized for loop
    for (u8 i = 0; i < count; i++) {
        if (fgets(buffer, buffer_size, file)) {
            // Remove newline character if present
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            // Create monster from line
            Monster::MONSTERS[i] = Monster(buffer, i);
            MONSTER_MAP[Monster::MONSTERS[i].name] = &Monster::MONSTERS[i];
        } else {
            // Failed to read line, break early
            break;
        }
    }
    
    // Clean up
    free(buffer);
    fclose(file);
}

void Monster::addMoves(const std::string &s) {
    for (const std::string &m : utils::split(s, ',')) {
        learnable.insert(Move::getMove(m));
    }
}

std::string Monster::toString() const {
    std::ostringstream ss;
    
    // Build type string from bit-packed types
    std::string typeStr = Types::TYPES[getType1()];
    u8 type2 = getType2();
    if (type2 != 15) { // 15 = no type
        typeStr += std::string("/") + Types::TYPES[type2];
    }
    
    ss << dexNum << ": " << name << " (" << typeStr << ")\n\tHP=" << hp << "\n\tAttack=" << atk << "\n\tDefense=" << def << "\n\tSpecial Attack=" << spatk
       << "\n\tSpecial Defense=" << spdef << "\n\tSpeed=" << spd;
    return ss.str();
}
