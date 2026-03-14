#include "Trainer.h"
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "Trainer.h"
#include "Monster.h"
#include "Move.h"
#include "Player.h"
#include "PokeMap.h"
#include "debug.h"
#include <string.h>
#include "sprites_data.hpp"

short Trainer::tid = 0;

Trainer::Trainer(const std::string &s) {
    std::vector<std::string> a = utils::split(s, ',');
    if (a[3] == "DOWN") {
        dir = 1;
    } else if (a[3] == "UP") {
        dir = 4;
    } else if (a[3] == "LEFT") {
        dir = 6;
    } else if (a[3] == "RIGHT") {
        dir = 8;
    } else {
        debugCheckpoint(s.c_str());
    }
    
    std::string spriteKey = a[0] + "_" + std::to_string(dir);
    bi = SPRITE_DATA[spriteKey];
    
    x = std::stoi(a[1]);
    y = std::stoi(a[2]);
    type = a[4];
    reward = std::stoi(a[5]);
    
    // Initialize optimized storage
    teamSize = 0;
    for (int i = 0; i < 6; i++) {
        mons[i] = nullptr;
        levels[i] = 0;
    }
    
    // Initialize phrases
    phrases = nullptr;
    phraseCount = 0;
    
    for (size_t i = 6; i < a.size(); i++) {
        //debugCheckpoint("START here");
        size_t j = a[i].find(' ');
        int level = std::stoi(a[i].substr(0, j));
        std::string monName = a[i].substr(j + 1);
        //debugCheckpoint((monName+"AhahaA\n"+Monster::MONSTER_MAP[monName]->name).c_str());
        
        // Store in optimized arrays instead of creating Battler objects
        if (teamSize < 6) {
            mons[teamSize] = Monster::MONSTER_MAP[monName];
            levels[teamSize] = level;
            teamSize++;
        }
    }
}

void Trainer::beat(Player *p) {
    beaten = true;
    p->trainersBeaten[id] = true;
}

Trainer::~Trainer() {
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

void Trainer::setPhrases(const std::string &quoteStr) {
    // Clean up existing phrases
    if (phrases) {
        for (int i = 0; i < phraseCount; i++) {
            if (phrases[i]) {
                delete[] phrases[i];
            }
        }
        delete[] phrases;
    }
    
    // Split and allocate new phrases
    std::vector<std::string> tempPhrases = utils::split(quoteStr, ';');
    phraseCount = tempPhrases.size();
    phrases = new char*[phraseCount];
    
    for (int i = 0; i < phraseCount; i++) {
        const std::string &phrase = tempPhrases[i];
        phrases[i] = new char[phrase.length() + 1]; // +1 for null terminator
        strcpy(phrases[i], phrase.c_str());
    }
}

const char* Trainer::getPhrase(int index) const {
    if (index < 0 || index >= phraseCount || !phrases[index]) {
        return ""; // Safety check
    }
    return phrases[index];
}

std::string Trainer::toString() const {
    return std::to_string(x) + " " + std::to_string(y);
}

Battler* Trainer::createBattler(int index) {
    if (index < 0 || index >= teamSize || !mons[index]) {
        return nullptr; // Invalid index
    }
    return new Battler(levels[index], mons[index]);
}

Battler** Trainer::createAllBattlers() {
    static Battler* result[6]; // Static to persist between calls
    for (int i = 0; i < 6; i++) {
        result[i] = (i < teamSize) ? createBattler(i) : nullptr;
    }
    return result;
}

Trainer::Leader::Leader(int id, const std::string &data, const std::string &tm) : Trainer(data) {
    this->id = id;
    move = Move::getMove(tm);
}

void Trainer::Leader::beat(Player *p) {
    beaten = true;
    p->leadersBeaten[id] = true;
    p->give(move);
}

Trainer::GioRival::GioRival(int id, const std::string &s) : Trainer(s) {
    this->id = id;
}

void Trainer::GioRival::beat(Player *p) {
    beaten = true;
    p->gioRivalsBeaten[id] = true;
}

Trainer::E4Trainer::E4Trainer(int id, const std::string &s) : Trainer(s) {
    this->id = id;
}

void Trainer::E4Trainer::beat(Player *p) {
    beaten = true;
    if (id == 8) {
        p->leadersBeaten[id] = true;
    }
}

void Trainer::addEliteFour() {
    const size_t buffer_size = 512;
    char* buffer = (char*)malloc(buffer_size);
    if (!buffer) {
        return; // Failed to allocate memory
    }
    
    FILE* file = fopen("nitro:/E4.txt", "r");
    if (!file) {
        free(buffer);
        return; // Failed to open file
    }
    
    PokeMap* pm = nullptr;
    std::string current_map = "";
    
    for(int i=0; i < 6&&fgets(buffer, buffer_size, file); i++) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        // Split by forward slash to separate trainer data from quote
        std::string line(buffer);
        size_t slash_pos = line.find('/');
        
        std::string trainer_data = line.substr(0, slash_pos);
        std::string quote = line.substr(slash_pos + 1);
        
        // Check if this line starts with a map name (contains semicolon before trainer data)
        size_t semicolon_pos = trainer_data.find(';');
        if (semicolon_pos != std::string::npos) {
            current_map = trainer_data.substr(0, semicolon_pos);
            trainer_data = trainer_data.substr(semicolon_pos + 1);
            pm = PokeMap::POKEMAPS[current_map];
        }
        
        // Add trainer to current map with quote
        pm->addTrainer(trainer_data, quote);
    }
    
    free(buffer);
    fclose(file);
}

void Trainer::buildTrainers() {
    const int total_trainers = 309; // First 309 lines are individual trainers
    const size_t buffer_size = 320; // Buffer for trainer data lines
    
    FILE* file = fopen("nitro:/Trainer.txt", "r");
    if (!file) {
        return; // Failed to open file
    }
    
    char* buffer = (char*)malloc(buffer_size);
    if (!buffer) {
        fclose(file);
        return; // Failed to allocate memory
    }
    
    PokeMap* pm = nullptr;
    std::string current_map = "";
    
    // Process first 309 individual trainers
    for (int i = 0; i < total_trainers; i++) {
        if (fgets(buffer, buffer_size, file)) {
            buffer[strlen(buffer) - 1] = '\0';
            //if(i%50==0)debugCheckpoint(buffer);
            // Split by forward slash to separate trainer data from quote
            std::string line(buffer);
            size_t slash_pos = line.find('/');
            
            std::string trainer_data = line.substr(0, slash_pos);
            std::string quote = line.substr(slash_pos + 1);
            
            // Check if this line starts with a map name (contains semicolon before trainer data)
            size_t semicolon_pos = trainer_data.find(';');
            if (semicolon_pos != std::string::npos) {
                current_map = trainer_data.substr(0, semicolon_pos);
                trainer_data = trainer_data.substr(semicolon_pos + 1);
                pm = PokeMap::POKEMAPS[current_map];
            }
            //debugCheckpoint((trainer_data+"\n"+quote+"\n").c_str());
            // Add trainer to current map with quote
            pm->addTrainer(trainer_data, quote);
        }
    }
    //debugCheckpoint("Got here");
    // Process next 9 leaders (always have semicolon - live on their own maps)
    for (int i = 0; i < 9; i++) {
        if (fgets(buffer, buffer_size, file)) {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            
            // Split by forward slash to separate leader data from TM
            std::string line(buffer);
            size_t slash_pos = line.find('/');
            
            std::string leader_data = line.substr(0, slash_pos);
            std::string quotes = line.substr(slash_pos + 1);
            //debugCheckpoint(buffer);
            std::vector<std::string> a = utils::split(leader_data, ';');
            PokeMap::POKEMAPS[a[0]]->addLeader(i, a[1], a[2], quotes);
        }
    }
    
    // Process last 9 gioRivals (always have semicolon - live on their own maps)
    for (int i = 0; i < 9; i++) {
        if (fgets(buffer, buffer_size, file)) {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            
            // Split by forward slash to separate gioRival data from quote
            std::string line(buffer);
            size_t slash_pos = line.find('/');
            if (slash_pos == std::string::npos) continue;
            
            std::string gioRival_data = line.substr(0, slash_pos);
            std::string quote = line.substr(slash_pos + 1);
            
            std::vector<std::string> a = utils::split(gioRival_data, ';');
            if (a.size() >= 2) {
                PokeMap::POKEMAPS[a[0]]->addGioRival(i, a[1], quote);
            }
        }
    }
    
    free(buffer);
    fclose(file);
    addEliteFour();
}

void Trainer::buildTrainers(Player *p, const std::string &trainS, const std::string &leadS, const std::string &gioRS) {
    
    // Read trainer data from Trainer.txt
    FILE* file = fopen("nitro:/Trainer.txt", "r");
    if (!file) {
        return; // Failed to open file
    }
    
    const size_t buffer_size = 320;
    char* buffer = (char*)malloc(buffer_size);
    if (!buffer) {
        fclose(file);
        return;
    }
    
    PokeMap* pm = nullptr;
    std::string current_map = "";
    u16 trainer_index = 0;
    // Process first 309 individual trainers
    for (int i = 0; i < 309; i++) {
        if (fgets(buffer, buffer_size, file)) {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            
            // Split by forward slash to separate trainer data from quote
            std::string line(buffer);
            size_t slash_pos = line.find('/');
            
            std::string trainer_data = line.substr(0, slash_pos);
            std::string quote = line.substr(slash_pos + 1);
            
            // Check if this line starts with a map name
            size_t semicolon_pos = trainer_data.find(';');
            if (semicolon_pos != std::string::npos) {
                current_map = trainer_data.substr(0, semicolon_pos);
                trainer_data = trainer_data.substr(semicolon_pos + 1);
                pm = PokeMap::POKEMAPS[current_map];
            }
            
            // Process multiple trainers on the same line
            std::vector<std::string> trainer_parts = utils::split(trainer_data, ';');
            for (const auto& part : trainer_parts) {
                if (trainer_index >= trainS.size()) {
                    throw std::runtime_error("Trainer save flags out of range: tid=" + std::to_string(trainer_index) +
                                             " size=" + std::to_string(trainS.size()));
                }
                if (trainS[trainer_index] == '1') {
                    p->trainersBeaten[i] = true;
                } else {
                    p->trainersBeaten[i] = false;
                    pm->addTrainer(part, quote);
                }
            }
        }
    }
    
    for (u8 i = 0; i < 9; i++) {
        if (fgets(buffer, buffer_size, file)) {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            
            std::string line(buffer);
            size_t slash_pos = line.find('/');
            
            std::string leader_data = line.substr(0, slash_pos);
            std::string quotes = line.substr(slash_pos + 1);
            
            std::vector<std::string> a = utils::split(leader_data, ';');
            if (leadS[i] == '1') {
                p->leadersBeaten[i] = true;
            } else {
                PokeMap::POKEMAPS[a[0]]->addLeader(i, a[1], a[2], quotes);
            }
        }
    }
    
    // Process last 9 gioRivals
    for (u8 i = 0; i < 9; i++) {
        if (fgets(buffer, buffer_size, file)) {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            
            std::string line(buffer);
            size_t slash_pos = line.find('/');
            if (slash_pos == std::string::npos) continue;
            
            std::string gioRival_data = line.substr(0, slash_pos);
            std::string quote = line.substr(slash_pos + 1);
            
            std::vector<std::string> a = utils::split(gioRival_data, ';');
            if (a.size() >= 2) {
                if (i >= gioRS.size()) {
                    throw std::runtime_error("GioRival save flags out of range: i=" + std::to_string(i) +
                                             " size=" + std::to_string(gioRS.size()));
                }
                if (gioRS[i] == '1') {
                    p->gioRivalsBeaten[i] = true;
                } else {
                    p->gioRivalsBeaten[i] = false;
                    PokeMap::POKEMAPS[a[0]]->addGioRival(i, a[1], quote);
                }
            }
        }
    }
    
    free(buffer);
    fclose(file);
    addEliteFour();
}
