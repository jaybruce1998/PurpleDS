#pragma once

#include "Monster.h"
#include <nds.h>

class Encounter {
public:
    static void buildEncounterRates();

    Monster *rMonster = nullptr;
    Monster *bMonster = nullptr;
    u8 chance = 0;
    u8 rLevel = 0;
    u8 bLevel = 0;

    Encounter() = default;
    explicit Encounter(const std::string &s);
    std::string toString() const;
};
