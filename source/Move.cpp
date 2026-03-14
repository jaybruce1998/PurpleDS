#include "Move.h"
#include "Utils.h"
#include "Types.h"
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <cstring>
#include "debug.h"

Move* Move::MOVES[175];

Move* Move::getMove(const std::string& name) {
    for (u8 i = 0; i < 175; i++) {
        if (MOVES[i]->name == name) {
            return MOVES[i];
        }
    }
    return nullptr;
}

void Move::buildMoves() {
    const int count = 175;
    const size_t buffer_size = 512; // Buffer for move data lines
    
    // Open file directly (nitrofs should be auto-initialized by build system)
    FILE* file = fopen("nitro:/Move.txt", "r");
    if (!file) {
        debugCheckpoint("No moves");
        return; // Failed to open file
    }
    
    // Allocate buffer on heap (not stack)
    char* buffer = (char*)malloc(buffer_size);
    if (!buffer) {
        fclose(file);
        return; // Failed to allocate memory
    }
    
    // Read exactly 175 lines using optimized for loop
    for (int i = 0; i < count; i++) {
        if (fgets(buffer, buffer_size, file)) {
            // Remove newline character if present
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            //debugCheckpoint(buffer);
            // Create move from line
            MOVES[i] = new Move(buffer);
        }
    }
    
    // Clean up
    free(buffer);
    fclose(file);
}

Move::Move(const std::string &s) {
    std::vector<std::string> a = utils::split(s, '\t');
    name = a[0];
    
    // Parse move type and attack type, pack into single byte
    u8 moveType = Types::getTypeIndex(a[1]);
    u8 attackType = STATUS;  // Initialize to default value
    if (a[2] == "Physical") {
        attackType = PHYSICAL;
    } else if (a[2] == "Special") {
        attackType = SPECIAL;
    }
    setTypes(moveType, attackType);
    
    power = (a[3] == "-" ? 0 : std::stoi(a[3]));
    acc = (a[4] == "-" ? 100000 : std::stoi(a[4]));
    pp = (a[5] == "-" ? 100000 : std::stoi(a[5]));
    if (a.size() == 6) {
        effects.clear();
    } else {
        effects = Effect::getEffects(utils::split(a[6], ' '), 0);
    }
}

Move::Move(const Move &m, int i) {
    name = m.name;
    type = m.type;
    power = m.power;
    acc = m.acc;
    pp = m.pp;
    for (; i < static_cast<int>(m.effects.size()); i++) {
        effects.push_back(m.effects[i]);
    }
}

Move::Move(const Move &m) : Move(m, 0) {
    if (!effects.empty()) {
        const Effect &e = effects[0];
        effects[0] = Effect(e.effect, e.amount + utils::randInt(0, e.variation) - 1);
    }
}

int Move::priority() const {
    if (effects.empty()) {
        return 0;
    }
    const Effect &e = effects[0];
    if (e.effect == "ATTACK_FIRST") {
        return 1;
    }
    if (e.effect == "ATTACK_SECOND") {
        return -1;
    }
    return 0;
}

int Move::critChance() const {
    if (effects.empty()) {
        return 1;
    }
    const Effect &e = effects[0];
    return e.effect == "CRIT_CHANCE" ? e.amount : 1;
}

bool Move::shouldPrintDamage() const {
    if (power == 0) {
        return false;
    }
    for (const Effect &e : effects) {
        if (e.effect == "CHARGE" || e.effect == "IMMUNE") {
            return false;
        }
    }
    return true;
}

std::string Move::toString() const {
    std::ostringstream ss;
    std::string moveTypeStr = Types::TYPES[getMoveType()];
    std::string atkTypeStr;
    switch (getAttackType()) {
        case STATUS: atkTypeStr = "Status"; break;
        case PHYSICAL: atkTypeStr = "Physical"; break;
        case SPECIAL: atkTypeStr = "Special"; break;
        default: atkTypeStr = "Unknown"; break;
    }
    ss << name << "(" << moveTypeStr << ")\n\tType=" << atkTypeStr << "\n\tPower=" << power << "\n\tAccuracy=" << acc
       << "\n\tPP=" << pp << "\n\tEffects=";
    for (const auto &e : effects) {
        ss << e.toString();
    }
    return ss.str();
}
