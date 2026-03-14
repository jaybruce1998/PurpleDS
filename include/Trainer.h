#pragma once

#include "Battler.h"
#include <nds.h>

class Player;

class Trainer {
public:
    static short tid;

    static void addEliteFour();
    static void buildTrainers();
    static void buildTrainers(Player *p, const std::string &trainS, const std::string &leadS, const std::string &gioRS);

    const u8* bi = nullptr;
    std::string type;
    u8 dir = 0;
    u8 x = 0;
    u8 y = 0;
    int reward = 0;
    u16 id = 0;
    bool beaten = false;
    char** phrases = nullptr; // Dynamic array of strings
    u8 phraseCount = 0; // Number of phrases
    
    // Optimized storage for team data
    Monster* mons[6];
    u8 levels[6];
    u8 teamSize = 0;

    Trainer() = default;
    explicit Trainer(const std::string &s);
    virtual ~Trainer();

    virtual void beat(Player *p);
    std::string toString() const;
    
    // Phrase management
    void setPhrases(const std::string &quoteStr);
    const char* getPhrase(int index) const;
    
    // Create Battlers on-demand from optimized storage
    Battler* createBattler(int index);
    Battler** createAllBattlers();

    class Leader;
    class GioRival;
    class E4Trainer;
};

class Trainer::Leader : public Trainer {
public:
    Move *move = nullptr;
    Leader(int id, const std::string &data, const std::string &tm);
    void beat(Player *p) override;
};

class Trainer::GioRival : public Trainer {
public:
    GioRival(int id, const std::string &s);
    void beat(Player *p) override;
};

class Trainer::E4Trainer : public Trainer {
public:
    E4Trainer(int id, const std::string &s);
    void beat(Player *p) override;
};
