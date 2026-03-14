#pragma once

#include "Move.h"
#include <nds.h>

class Types {
public:
    static const char* TYPES[15];
    static const u8 TYPE_CHART[15][15];

    static void buildTypes();
    static u8 getTypeIndex(const std::string& typeName);
    static double damage(const Move &move, const std::vector<std::string> &types);
    static double damage(const Move &move, u8 types); // New overload for bit-packed types
};
