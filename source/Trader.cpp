#include "Trader.h"
#include "Gui.h"
#include "PokeMap.h"
#include "Battler.h"
#include "Player.h"
#include "Utils.h"
#include "Item.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include "debug.h"

// External global gui instance
extern Gui* g_gui;

void Trader::buildTraders() {
    const size_t buffer_size = 256; // Buffer for trader data lines
    
    FILE* file = fopen("nitro:/Trader.txt", "r");
    if (!file) {
        return; // Failed to open file
    }
    
    char* buffer = (char*)malloc(buffer_size);
    if (!buffer) {
        fclose(file);
        return; // Failed to allocate memory
    }
    
    // Process first 6 dex entries (lines 0-5)
    for (int i = 0; i < 6; i++) {
        if (fgets(buffer, buffer_size, file)) {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            
            std::vector<std::string> a = utils::split(buffer, ';');
            if (a.size() >= 2) {
                PokeMap::POKEMAPS[a[0]]->addNpc(a[1], "Oh wow, another pokedex! Nifty.");
            }
        } else {
            break;
        }
    }
    
    // Process next 5 clipboard entries (lines 6-10)
    for (int i = 6; i < 11; i++) {
        if (fgets(buffer, buffer_size, file)) {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            
            std::vector<std::string> a = utils::split(buffer, ';');
            if (a.size() >= 2) {
                PokeMap::POKEMAPS[a[0]]->addNpc(a[1], "Wait, I can't read!");
            }
        } else {
            break;
        }
    }
    
    // Process next 11 monster entries (lines 11-21)
    for (int i = 11; i < 22; i++) {
        if (fgets(buffer, buffer_size, file)) {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            
            std::vector<std::string> a = utils::split(buffer, ';');
            if (a.size() >= 2) {
                PokeMap::POKEMAPS[a[0]]->addNpc(a[1], "Meow!");
            }
        } else {
            break;
        }
    }
    
    // Process next 6 fairy entries (lines 22-27)
    for (int i = 22; i < 28; i++) {
        if (fgets(buffer, buffer_size, file)) {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            
            std::vector<std::string> a = utils::split(buffer, ';');
            if (a.size() >= 2) {
                PokeMap::POKEMAPS[a[0]]->addNpc(a[1], "Hello...");
            }
        } else {
            break;
        }
    }
    
    // Process next 7 bird entries (lines 28-34)
    for (int i = 28; i < 35; i++) {
        if (fgets(buffer, buffer_size, file)) {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            
            std::vector<std::string> a = utils::split(buffer, ';');
            if (a.size() >= 2) {
                PokeMap::POKEMAPS[a[0]]->addNpc(a[1], "SCREECH!");
            }
        } else {
            break;
        }
    }
    
    while (fgets(buffer, buffer_size, file)) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        
        std::vector<std::string> a = utils::split(buffer, ';');
        if (a.size() >= 3) {
            PokeMap::POKEMAPS[a[0]]->addTrader(a[1], a[2]);
        }
    }
    
    free(buffer);
    fclose(file);
}

Trader::Trader(const std::string &s, const std::string &p) : Giver(s + ",null", "") {
    std::vector<std::string> a = utils::split(p, ',');
    monA = Monster::MONSTER_MAP[a[0]];
    monB = Monster::MONSTER_MAP[a[1]];
}

void Trader::interact(Player *p) {
    if (item == nullptr) {
        if (p->team[0] && p->team[0]->name == monA->name) {
            for (int i = 1; i < 6; i++) {
                p->team[i - 1] = p->team[i];
            }
            p->team[5] = nullptr;
            p->give(new Battler(1, monB));
            g_gui->print("Yoink!");
        } else {
            g_gui->print("Don't talk to me unless you're leading with " + monA->name + "!");
        }
    } else if (p->hasItem(item)) {
        p->use(item);
        p->give(new Battler(1, monB));
        g_gui->print("Yoink!");
    } else {
        g_gui->print("Don't talk to me unless you have " + item->name + "!");
    }
}
