#pragma once

#include "Giver.h"
#include "Monster.h"

class Trader : public Giver {
public:
    static void buildTraders();

    Monster *monA = nullptr;
    Monster *monB = nullptr;

    Trader() = default;
    Trader(const std::string &s, const std::string &p);
    void interact(Player *p) override;
};
