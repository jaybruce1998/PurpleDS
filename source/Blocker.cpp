#include "Blocker.h"
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

void Blocker::buildBlockers() {
    // Process first 28 lines from Blocker.txt (forward slash separated)
    const int blocker_count = 28;
    const size_t buffer_size = 1024; // Buffer for blocker data lines
    
    FILE* file = fopen("nitro:/Blocker.txt", "r");
    if (!file) {
        debugCheckpoint("No blockers");
        return; // Failed to open file
    }
    
    char* buffer = (char*)malloc(buffer_size);
    if (!buffer) {
        fclose(file);
        return; // Failed to allocate memory
    }
    
    // Read first 28 blocker lines
    for (int i = 0; i < blocker_count; i++) {
        if (fgets(buffer, buffer_size, file)) {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            
            // Split by forward slash to separate blocker data from quotes
            std::string line(buffer);
            size_t slash_pos = line.find('/');
            
            std::string blocker_data = line.substr(0, slash_pos);
            std::string quotes = line.substr(slash_pos + 1);
            
            // Process blocker data - split by semicolon
            std::vector<std::string> a = utils::split(blocker_data, ';');
            PokeMap::POKEMAPS[a[0]]->addBlocker(a[1], quotes);
        } else {
            break;
        }
    }
    // Process next 16 badge blocker lines
    for (int i = 0; i < 16; i++) {
        if (fgets(buffer, buffer_size, file)) {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            std::string line(buffer);
            size_t slash_pos = line.find('/');
            std::string blocker_data = line.substr(0, slash_pos);
            std::string quotes = line.substr(slash_pos + 1);

            //debugCheckpoint(buffer);
            std::vector<std::string> a = utils::split(blocker_data, ';');
            PokeMap::POKEMAPS[a[0]]->addBlocker(a[1], quotes, std::stoi(a[2]));
        } else {
            break;
        }
    }
    // Process remaining boulder entries from Blocker.txt (share same quote)
    while (fgets(buffer, buffer_size, file)) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        
        std::vector<std::string> a = utils::split(buffer, ';');
        PokeMap *pm = PokeMap::POKEMAPS[a[0]];
        for (size_t i = 1; i < a.size(); i++) {
            pm->addBlocker(a[i], "Ugh, this is too heavy!;Heh, light as a feather!", 3);
        }
    }
    
    free(buffer);
    fclose(file);
    
    // Route23 boulders stay in cpp (hardcoded)
    PokeMap *pm = PokeMap::POKEMAPS["Route23"];
    for (u8 i = 104; i < 108; i += 2) {
        for (u8 j = 10; j < 16; j++) {
            pm->addBlocker("BOULDER " + std::to_string(j) + " " + std::to_string(i) + " DOWN,Move=Strength",
                           "Ugh, this is too heavy!;Heh, light as a feather!", 3);
        }
    }
}

Blocker::Blocker(const std::string &s, const std::string &q) : Giver(s, q), numBadges(-1) {}

Blocker::Blocker(const std::string &s, const std::string &q, int n) : Giver(s, q), numBadges(n) {}

void Blocker::interact(Player *p) {
    bool missingItem = false;
    if (item != nullptr) {
        if (item->name == "Master Ball") {
            missingItem = p->ballin;
        } else {
            missingItem = !p->hasItem(item);
        }
    }
    if ((move != nullptr && !p->hasMove(move)) || (numBadges >= 0 && !p->leadersBeaten[numBadges]) || missingItem) {
        g_gui->print(getPhrase(0));
        return;
    }
    dead = true;
    g_gui->print(getPhrase(1));
}
