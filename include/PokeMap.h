#pragma once

#include "Utils.h"
#include "map_data.h"

class Warp;
class Encounter;
class Trainer;
class WorldObject;
class Npc;
class MartItem;
class Battler;
class Player;

class PokeMap {
public:
    static u8 TILE_TYPES[801];
    static std::unordered_map<std::string, PokeMap*> POKEMAPS;

    static void buildPokeMaps();

    MapInfo* mapInfo;
    PokeMap *north = nullptr;
    PokeMap *south = nullptr;
    PokeMap *east = nullptr;
    PokeMap *west = nullptr;
    u8 nOff = 0;
    u8 sOff = 0;
    u8 eOff = 0;
    u8 wOff = 0;
    u8 healX = 0;
    u8 healY = 0;
    std::string name;
    std::unordered_map<short, Warp*> warps;
    std::unordered_map<std::string, std::vector<Encounter>> encounters;
    Trainer** trainers;
    s8* sight;
    WorldObject** wob;
    Npc** npcs;
    std::unordered_map<int, std::vector<MartItem>> martMap;

    PokeMap(const std::string &name, MapInfo* mapInfo);
    ~PokeMap();

    // Helper methods to access grid and types data from mapInfo
    int getGridValue(int row, int col) const;
    int getGridRows() const { return mapInfo->rows; }
    int getGridCols() const { return mapInfo->cols; }
    int getTypeValue(int row, int col) const;

    Trainer* getTrainer(int x, int y, int i);
    void deleteTrainer(Trainer *t, int i);
    void addConnection(char d, PokeMap *pm, int off);
    void addWarp(int row, int col, Warp *w);
    Warp* getWarp(int row, int col);
    std::vector<MartItem>* getMartItems(u8 x, u8 y);
    Warp* getNearbyWarp(int row, int col);
    void addEncounters(const std::vector<std::string> &a);
    Battler* getRandomEncounter(const std::string &type);
    void addTrainer(const std::string &s, const std::string &quote);
    void addTrainers(const std::vector<std::string> &a, const std::string &quote);
    void addLeader(int i, const std::string &data, const std::string &move, const std::string &quotes);
    void addGioRival(int i, const std::string &s, const std::string &quote);
    void addE4Trainer(int id, const std::string &s, const std::string &p);
    void addItem(const std::string &s);
    void addItems(const std::vector<std::string> &a);
    void addEncounter(const std::string &s);
    void addSnorlaxEncounter(const std::string &s);
    void addGiver(const std::string &s, const std::string &q);
    void addGiver(const std::string &s, int n, const std::string &q);
    void addGiver(const std::string &s, const std::string &i, const std::string &q);
    void addMartItems(const std::vector<std::string> &a);
    void addBlocker(const std::string &s, const std::string &q);
    void addBlocker(const std::string &s, const std::string &q, int n);
    void setHeal(int n);
    void addNpc(const std::string &s, const std::string &q);
    void addTrader(const std::string &s, const std::string &p);
    void stepOn(Player *p, int x, int y);

    std::string toString() const;
};
