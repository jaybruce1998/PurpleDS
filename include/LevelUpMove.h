#pragma once

#include "Move.h"

class LevelUpMove {
public:
    static std::vector<LevelUpMove> LEVEL_UP_MOVES[153];

    static void buildLevelUpMoves();

    u8 level = 0;
    Move *move = nullptr;

    LevelUpMove() = default;
    explicit LevelUpMove(const std::string &s);
    std::string toString() const;
};
