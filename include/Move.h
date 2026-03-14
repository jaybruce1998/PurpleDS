#pragma once

#include "Effect.h"
#include <nds.h>

class Move {
public:
    static Move* MOVES[175];

    static void buildMoves();
    static Move* getMove(const std::string& name);

    std::string name;
    u8 type = 0; // Bit-packed: low 4 bits = move type, high 4 bits = attack type (0=STATUS, 1=PHYSICAL, 2=SPECIAL)
    short power = 0;
    short acc = 0;
    u8 pp = 0;
    std::vector<Effect> effects;

    Move() = default;
    explicit Move(const std::string &s);
    Move(const Move &m, int i);
    explicit Move(const Move &m);

    int priority() const;
    int critChance() const;
    bool shouldPrintDamage() const;
    std::string toString() const;
    
    // Type helper methods
    u8 getMoveType() const { return type & 0x0F; }
    u8 getAttackType() const { return (type >> 4) & 0x0F; }
    void setTypes(u8 moveType, u8 attackType) { type = (attackType << 4) | (moveType & 0x0F); }
    
    // Attack type constants
#define STATUS 0
#define PHYSICAL 1
#define SPECIAL 2
};
