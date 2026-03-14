#pragma once

#include "Giver.h"

class Blocker : public Giver {
public:

    static void buildBlockers();

    signed char numBadges = -1;

    Blocker() = default;
    Blocker(const std::string &s, const std::string &q);
    Blocker(const std::string &s, const std::string &q, int n);
    void interact(Player *p) override;
};
