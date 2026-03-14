#include "Npc.h"
#include "PokeMap.h"
#include "Gui.h"
#include "Player.h"
#include "Trainer.h"
#include "Utils.h"
#include "sprites_data.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include "debug.h"

static Move *FLASH = nullptr;

void Npc::buildNpcs() {
    FLASH = Move::getMove("Flash");
    
    const short total_npcs = 311;
    const size_t buffer_size = 512; // Buffer for NPC data lines
    
    FILE* file = fopen("nitro:/Npc.txt", "r");
    if (!file) {
        return; // Failed to open file
    }
    
    char* buffer = (char*)malloc(buffer_size);
    if (!buffer) {
        fclose(file);
        return; // Failed to allocate memory
    }
    PokeMap* pm = nullptr;
    for (short i = 0; i < total_npcs; i++) {
        if (fgets(buffer, buffer_size, file)) {
            buffer[strlen(buffer) - 1] = '\0';
            //if(i%50==0)debugCheckpoint(buffer);
            
            // Split by forward slash to separate NPC data from quote
            std::string line(buffer);
            size_t slash_pos = line.find('/');
            
            std::string npc_data = line.substr(0, slash_pos);
            std::string quote = line.substr(slash_pos + 1);
            
            // Check if NPC data has semicolons (multiple NPCs)
            if (npc_data.find(';') != std::string::npos) {
                std::vector<std::string> a = utils::split(npc_data, ';');
                pm = PokeMap::POKEMAPS[a[0]];
                npc_data=a[1];
            }
            pm->addNpc(npc_data, quote);
        } else {
            break;
        }
    }
    
    free(buffer);
    fclose(file);
}

Npc::Npc(const std::string &s, const std::string &q) {
    std::vector<std::string> f = utils::split(s, ' ');
    u8 dir = 0;
    
    // Parse coordinates first
    if (f.size() >= 3) {
        try {
            x = std::stoi(f[1]);
            y = std::stoi(f[2]);
        } catch (...) {
            x = 0;
            y = 0;
        }
    } else {
        x = 0;
        y = 0;
    }
    
    // Parse direction
    if (f[3] == "DOWN") {
        dir = 0;
    } else if (f[3] == "UP") {
        dir = 1;
    } else if (f[3] == "LEFT") {
        dir = 2;
    } else if (f[3] == "RIGHT") {
        dir = 3;
    } else {
        dir = 0; // Default to DOWN
    }
    
    // For 3DS, use sprite map to get raw sprite data
    std::string spriteKey = f[0] + "_" + std::to_string(dir);
    
    // Check if sprite exists in map
    bi = SPRITE_DATA[spriteKey];
    
    setPhrases(q);
}

void Npc::interact(Player *p) {
    if (p->hasMove(FLASH) && utils::rand01() < 0.01) {
        Gui::spacebar = false;
        g_gui->print(getPhrase(0));
        g_gui->print(p->name + " used flash!");
        if (utils::rand01() < 0.5) {
            g_gui->print("NOO PLEASE DON'T!!!");
            g_gui->print("(They faded into the void...)");
            dead = true;
        } else {
            g_gui->print("Well in that case...");
            g_gui->print("They flashed you back!");
        }
    } else {
        g_gui->print(getPhrase(0));
    }
}

Npc::~Npc() {
    // Clean up dynamic phrases array
    if (phrases) {
        for (int i = 0; i < phraseCount; i++) {
            if (phrases[i]) {
                delete[] phrases[i]; // Delete each string
            }
        }
        delete[] phrases; // Delete the array of pointers
        phrases = nullptr;
        phraseCount = 0;
    }
}

void Npc::setPhrases(const std::string &quoteStr) {
    // Split and allocate new phrases
    std::vector<std::string> tempPhrases = utils::split(quoteStr, ';');
    phraseCount = tempPhrases.size();
    phrases = new char*[phraseCount];
    
    for (u8 i = 0; i < phraseCount; i++) {
        const std::string &phrase = tempPhrases[i];
        phrases[i] = new char[phrase.length() + 1]; // +1 for null terminator
        strcpy(phrases[i], phrase.c_str());
    }
}

const char* Npc::getPhrase(int index) const {
    if (index < 0 || index >= phraseCount || !phrases[index]) {
        return ""; // Safety check
    }
    return phrases[index];
}
