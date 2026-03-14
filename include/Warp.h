#pragma once

#include "PokeMap.h"

class Warp {
public:
    static void buildWarps();

    PokeMap *pm = nullptr;
    u8 row = 0;
    u8 col = 0;

    Warp() = default;
    Warp(PokeMap *pm, int row, int col);
    std::string toString() const;
};
