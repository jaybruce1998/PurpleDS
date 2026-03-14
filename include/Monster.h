#pragma once

#include "Move.h"
#include <nds.h>

class Monster {
public:
    static Monster MONSTERS[153];
    static std::unordered_map<std::string, Monster*> MONSTER_MAP;

    static void buildMonsters();

    std::string name;
    u8 types = 0; // Bit-packed: low 4 bits = type1, high 4 bits = type2 (15 = no type)
    int hp = 0;
    int atk = 0;
    int def = 0;
    int spatk = 0;
    int spdef = 0;
    int spd = 0;
    u8 dexNum = 0;
    std::unordered_set<Move*> learnable;

    Monster();
    Monster(const std::string &s, int dexNum);
    void addMoves(const std::string &s);
    std::string toString() const;
    
    // Type helper methods
    u8 getType1() const { return types & 0x0F; }
    u8 getType2() const { return (types >> 4) & 0x0F; }
    void setTypes(u8 type1, u8 type2) { types = (type2 << 4) | (type1 & 0x0F); }
};
