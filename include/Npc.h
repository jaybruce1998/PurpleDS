#pragma once

#include "Move.h"
#include <nds.h>

class Player;

class Npc {
public:
    static void buildNpcs();

    const u8* bi = nullptr;
    u8 x = 0;
    u8 y = 0;
    char** phrases = nullptr; // Dynamic array of strings
    u8 phraseCount = 0; // Number of phrases
    bool dead = false;

    Npc() = default;
    Npc(const std::string &s, const std::string &q);
    virtual ~Npc();

    virtual void interact(Player *p);
    
    // Phrase management
    void setPhrases(const std::string &quoteStr);
    const char* getPhrase(int index) const;
};
