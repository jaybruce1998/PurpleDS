#include "Giver.h"
#include "Gui.h"
#include "PokeMap.h"
#include "Player.h"
#include "Utils.h"
#include "debug.h"
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

// External global gui instance
extern Gui* g_gui;

u8 Giver::gid = 0;

void Giver::buildGivers() {
    const int count = 27;
    const size_t buffer_size = 1024; // Buffer for giver data lines (larger due to quotes)
    
    // Open file directly (nitrofs should be auto-initialized by build system)
    FILE* file = fopen("nitro:/Giver.txt", "r");
    if (!file) {
        return; // Failed to open file
    }
    
    // Allocate buffer on heap (not stack)
    char* buffer = (char*)malloc(buffer_size);
    if (!buffer) {
        fclose(file);
        return; // Failed to allocate memory
    }
    
    // Read exactly 27 lines using optimized for loop
    for (int i = 0; i < count; i++) {
        if (fgets(buffer, buffer_size, file)) {
            // Remove newline character if present
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            
            // Split by forward slash to separate giver data from quotes
            std::string line(buffer);
            size_t slash_pos = line.find('/');
            if (slash_pos == std::string::npos) continue; // Skip malformed lines
            
            std::string giver_data = line.substr(0, slash_pos);
            std::string quotes = line.substr(slash_pos + 1);
            
            // Process giver data - split by semicolon
            std::vector<std::string> a = utils::split(giver_data, ';');
            
            // Add giver based on index
            if (i < 5) {
                // First batch: uses std::stoi for third parameter
                PokeMap::POKEMAPS[a[0]]->addGiver(a[1], std::stoi(a[2]), quotes);
            } else if (i < 8) {
                // Second batch: uses string for third parameter
                PokeMap::POKEMAPS[a[0]]->addGiver(a[1], a[2], quotes);
            } else {
                // Third batch: only giver data and quotes
                PokeMap::POKEMAPS[a[0]]->addGiver(a[1], quotes);
            }
        } else {
            // Failed to read line, break early
            break;
        }
    }
    
    // Clean up
    free(buffer);
    fclose(file);
}

Giver::Giver(const std::string &s, const std::string &q) : Npc(utils::split(s, ',')[0], q) {
    std::vector<std::string> a = utils::split(s, ',');
    id = gid++;
    if (a.size() > 1) {
        if (a[1].rfind("Move", 0) == 0) {
            move = Move::getMove(a[1].substr(5));
        } else {
            item = Item::getItem(a[1]);
        }
    }
}

void Giver::interact(Player *p) {
    if (!dead&&(!item||item->name!="Master Ball"||p->ballin)) {
        g_gui->print(getPhrase(0));
        if (move == nullptr) {
            p->give(item);
        } else {
            p->give(move);
        }
        dead = true;
    }
    g_gui->print(getPhrase(1));
}

Giver::Aide::Aide(const std::string &s, int dexNum, const std::string &q) : Giver(s, q), dexNum(dexNum) {}

void Giver::Aide::interact(Player *p) {
    if (p->numCaught < dexNum) {
        g_gui->print(getPhrase(0));
        return;
    }
    if (!dead) {
        g_gui->print(getPhrase(1));
        if (move == nullptr) {
            if (item && item->name == "Shiny Charm") {
                Player::SHINY_CHANCE = 256;
            }
            p->give(item);
        } else {
            p->give(move);
        }
        dead = true;
    }
    g_gui->print(getPhrase(2));
}

Giver::IfGiver::IfGiver(const std::string &s, const std::string &i, const std::string &q) : Giver(s, q) {
    preReq = Item::getItem(i);
}

void Giver::IfGiver::interact(Player *p) {
    if (!p->hasItem(preReq)) {
        g_gui->print(getPhrase(0));
        return;
    }
    if (!dead) {
        g_gui->print(getPhrase(1));
        if (move == nullptr) {
            p->give(item);
        } else {
            p->give(move);
        }
        dead = true;
    }
    g_gui->print(getPhrase(2));
}
