#include "Types.h"

const char* Types::TYPES[15] = {"Normal", "Fire", "Water", "Electric", "Grass", "Ice", "Fighting", "Poison", "Ground",
                                 "Flying", "Psychic", "Bug", "Rock", "Ghost", "Dragon"};
const u8 Types::TYPE_CHART[15][15] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 0, 1},
    {1, 5, 5, 1, 2, 2, 1, 1, 1, 1, 1, 2, 5, 1, 5},
    {1, 2, 5, 1, 5, 1, 1, 1, 2, 1, 1, 1, 2, 1, 5},
    {1, 1, 2, 5, 5, 1, 1, 1, 0, 2, 1, 1, 1, 1, 5},
    {1, 5, 2, 1, 5, 1, 1, 5, 2, 5, 1, 5, 2, 1, 5},
    {1, 1, 5, 1, 2, 5, 1, 1, 2, 2, 1, 1, 1, 1, 2},
    {2, 1, 1, 1, 1, 2, 1, 5, 1, 5, 5, 5, 2, 0, 1},
    {1, 1, 1, 1, 2, 1, 1, 5, 5, 1, 1, 2, 5, 5, 1},
    {1, 2, 1, 2, 5, 1, 1, 2, 1, 0, 1, 5, 2, 1, 1},
    {1, 1, 1, 5, 2, 1, 2, 1, 1, 1, 1, 2, 5, 1, 1},
    {1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 5, 1, 1, 1, 1},
    {1, 5, 1, 1, 2, 1, 5, 2, 1, 5, 2, 1, 1, 5, 1},
    {1, 2, 1, 1, 1, 2, 5, 1, 5, 2, 1, 2, 1, 1, 1},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
};

u8 Types::getTypeIndex(const std::string& typeName) {
    for (u8 i = 0; i < 15; i++) {
        if (TYPES[i] == typeName) {
            return i;
        }
    }
    return 15; // Return "no type" index if not found
}

double Types::damage(const Move &move, const std::vector<std::string> &types) {
    double d = 1.0;
    u8 moveType = move.getMoveType();
    if (moveType == 15) { // 15 = ??? type
        return d;
    }
    u8 i = moveType;
    for (const auto &t : types) {
        u8 effectiveness = TYPE_CHART[i][getTypeIndex(t)];
        switch (effectiveness) {
            case 0: return effectiveness;
            case 1: continue;
            case 2: d *= 2; break;
            case 5: d /= 2; break;
        }
    }
    return d;
}

double Types::damage(const Move &move, u8 types) {
    double d = 1.0;
    u8 moveType = move.getMoveType();
    if (moveType == 15) { // 15 = ??? type
        return d;
    }
    u8 i = moveType;
    
    // Check type1 (low 4 bits)
    u8 type1 = types & 0x0F;
    if (type1 != 15) { // 15 = no type
        u8 effectiveness = TYPE_CHART[i][type1];
        switch (effectiveness) {
            case 0: return effectiveness;
            case 2: d *= 2; break;
            case 5: d /= 2; break;
        }
    }
    
    // Check type2 (high 4 bits)
    u8 type2 = (types >> 4) & 0x0F;
    if (type2 != 15) { // 15 = no type
        u8 effectiveness = TYPE_CHART[i][type2];
        switch (effectiveness) {
            case 0: return effectiveness;
            case 2: d *= 2; break;
            case 5: d /= 2; break;
        }
    }
    
    return d;
}
