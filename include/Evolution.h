#pragma once

#include "Monster.h"
#include <nds.h>

class Evolution {
public:
    static Evolution* EVOLUTIONS[153];

    static void buildEvolutions();

    std::string stone;
    u8 level = 0;
    u8 evo = 0;

    Evolution() = default;
    Evolution(const std::string &stone, int evo);
    Evolution(int level, int evo);
    std::string toString() const;
};
