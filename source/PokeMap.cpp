#include "PokeMap.h"
#include "map_data.h"
#include "Encounter.h"
#include "Trainer.h"
#include "Warp.h"
#include "WorldObject.h"
#include "Giver.h"
#include "Blocker.h"
#include "Trader.h"
#include "Npc.h"
#include "MartItem.h"
#include "Battler.h"
#include "Player.h"
#include "debug.h"
#include "Gui.h"
#include <iostream>

u8 PokeMap::TILE_TYPES[801] = {0, 8, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 2, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 2, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 3, 7, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 1, 4, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 5, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 2, 1, 1, 0, 1, 6, 2, 0, 0, 1, 0, 0, 0, 1, 0, 1, 2, 0, 7, 0, 7, 1, 0, 0, 0, 1, 0, 0, 0, 7, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 7, 1, 1, 0, 0, 0, 0, 20, 0, 9, 0, 0, 0, 0, 0, 0, 0, 8, 7, 0, 0, 8, 0, 0, 8, 0, 4, 0, 0, 0, 7, 0, 0, 2, 0, 0, 0, 0, 0, 1, 1, 7, 1, 5, 5, 1, 1, 1, 1, 1, 1, 1, 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 2, 2, 4, 1, 4, 4, 0, 0, 0, 0, 1, 0, 7, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 1, 4, 4, 0, 
	0, 0, 0, 0, 10, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 0, 1, 0, 0, 0, 0, 0, 0, 7, 0, 0, 7, 11, 0, 0, 0, 0, 3, 19, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 12, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 7, 8, 0, 0, 7, 0, 0, 0, 0, 0, 0, 13, 0, 1, 14, 16, 17, 15, 0, 0, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 18, 18, 0, 0, 0, 0, 0, 0, 0, 1, 4, 2, 2, 0, 7, 0, 0, 1, 0, 0, 12, 4, 4, 4, 4, 4, 0, 
	0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 2, 0, 0, 12, 1, 1, 12, 12, 12, 4, 4, 18, 4, 1, 4, 18, 18, 1, 18, 4, 4, 0, 0, 0, 0, 0, 0, 7, 0, 0, 3, 0, 0, 0, 9, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 
	2, 2, 7, 7, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 7, 0, 0, 0, 1, 0, 1, 0, 0, 0, 4, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 0, 0, 21, 8, 0, 0, 0, 0, 0, 0,
	2, 18, 18, 2, 18, 18, 2, 18, 18, 2, 2, 18, 18, 2, 6, 2, 18, 2, 2, 12};

std::unordered_map<std::string, PokeMap*> PokeMap::POKEMAPS;

void PokeMap::buildPokeMaps() {
    
    // Create PokeMap objects from the global map data
    initializeAllMaps();
    
    POKEMAPS["RedsHouse1F"]->setHeal(5);
    POKEMAPS["IndigoPlateauLobby"]->setHeal(7);
    
    // Read PokeMap connections from file
    FILE* file = fopen("nitro:/PokeMap.txt", "r");
    if (!file) {
        throw std::runtime_error("Failed to open PokeMap.txt");
    }
    
    const int buffer_size = 256;
    char* buffer = (char*)malloc(buffer_size);
    if (!buffer) {
        fclose(file);
        throw std::runtime_error("Failed to allocate memory for PokeMap.txt");
    }
    
    for (int i = 0; i < 78; i++) {
        if (fgets(buffer, buffer_size, file)) {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            
            std::string s(buffer);
            std::vector<std::string> a = utils::split(s, ' ');
            if (a.size() < 4) {
                throw std::runtime_error("Bad connection string: " + s);
            }
            if (!POKEMAPS.count(a[0]) || !POKEMAPS.count(a[2])) {
                throw std::runtime_error("Missing map in connection: " + s);
            }
            POKEMAPS[a[0]]->addConnection(a[1][0], POKEMAPS[a[2]], std::stoi(a[3]));
        }
    }
    
    free(buffer);
    fclose(file);
}

PokeMap::PokeMap(const std::string &name, MapInfo* mapInfo) : mapInfo(mapInfo), name(name) {
    int totalSize = mapInfo->rows * mapInfo->cols;
    
    // Allocate 1D arrays using calloc for zero initialization
    sight = (s8*)calloc(totalSize, sizeof(s8));
    trainers = (Trainer**)calloc(totalSize, sizeof(Trainer*));
    wob = (WorldObject**)calloc(totalSize, sizeof(WorldObject*));
    npcs = (Npc**)calloc(totalSize, sizeof(Npc*));
    
    healX = name.size() >= 10 && name.rfind("Pokecenter") == name.size() - 10 ? 3 : -2;
    healY = healX;
}

PokeMap::~PokeMap() {
    // Clean up allocated arrays
    free(sight);
    free(trainers);
    free(wob);
    free(npcs);
}

int PokeMap::getGridValue(int row, int col) const {
    return mapInfo->data[row * mapInfo->cols + col];
}

int PokeMap::getTypeValue(int row, int col) const {
    int tileId = getGridValue(row, col);
    return TILE_TYPES[tileId];
}

Trainer* PokeMap::getTrainer(int x, int y, int i) {
    int index = y * mapInfo->cols + x;
    if (trainers[index] != nullptr) {
        return trainers[index];
    }
    for (int r = y - 1; r >= 0; r--) {
        int sightIndex = r * mapInfo->cols + x;
        if (sight[sightIndex] != i) {
            break;
        } else if (trainers[sightIndex] != nullptr) {
            return trainers[sightIndex];
        }
    }
    for (int r = y + 1; r < mapInfo->rows; r++) {
        int sightIndex = r * mapInfo->cols + x;
        if (sight[sightIndex] != i) {
            break;
        } else if (trainers[sightIndex] != nullptr) {
            return trainers[sightIndex];
        }
    }
    for (int c = x - 1; c >= 0; c--) {
        int sightIndex = y * mapInfo->cols + c;
        if (sight[sightIndex] != i) {
            break;
        } else if (trainers[sightIndex] != nullptr) {
            return trainers[sightIndex];
        }
    }
    for (int c = x + 1; c < mapInfo->cols; c++) {
        int sightIndex = y * mapInfo->cols + c;
        if (sight[sightIndex] != i) {
            break;
        } else if (trainers[sightIndex] != nullptr) {
            return trainers[sightIndex];
        }
    }
    sight[index] = 0;
    return nullptr;
}

void PokeMap::deleteTrainer(Trainer *t, int i) {
    int y = t->y;
    int x = t->x;
    int index = y * mapInfo->cols + x;
    trainers[index] = nullptr;
    sight[index] = 0;
    
    // Clean up trainer's team to prevent memory leaks
    /*std::vector<Battler*> teamBattlers = t->createAllBattlers();
    for (Battler* b : teamBattlers) {
        delete b;
    }*/
    
    // Delete the trainer object itself
    delete t;
    
    for (int r = y - 1; r >= 0; r--) {
        int sightIndex = r * mapInfo->cols + x;
        if (sight[sightIndex] != i) {
            break;
        } else {
            sight[sightIndex] = 0;
        }
    }
    for (int r = y + 1; r < mapInfo->rows; r++) {
        int sightIndex = r * mapInfo->cols + x;
        if (sight[sightIndex] != i) {
            break;
        } else {
            sight[sightIndex] = 0;
        }
    }
    for (int c = x - 1; c >= 0; c--) {
        int sightIndex = y * mapInfo->cols + c;
        if (sight[sightIndex] != i) {
            break;
        } else {
            sight[sightIndex] = 0;
        }
    }
    for (int c = x + 1; c < mapInfo->cols; c++) {
        int sightIndex = y * mapInfo->cols + c;
        if (sight[sightIndex] != i) {
            break;
        } else {
            sight[sightIndex] = 0;
        }
    }
}

void PokeMap::addConnection(char d, PokeMap *pm, int off) {
    switch (d) {
        case 'n':
            north = pm;
            nOff = off;
            break;
        case 's':
            south = pm;
            sOff = off;
            break;
        case 'e':
            east = pm;
            eOff = off;
            break;
        case 'w':
            west = pm;
            wOff = off;
            break;
        default:
            throw std::runtime_error("wtf is " + std::string(1, d) + "?!");
    }
}

void PokeMap::addWarp(int row, int col, Warp *w) {
    warps[row * static_cast<int>(mapInfo->cols) + col] = w;
}

Warp* PokeMap::getWarp(int row, int col) {
    int key = row * static_cast<int>(mapInfo->cols) + col;
    auto it = warps.find(key);
    return it == warps.end() ? nullptr : it->second;
}

std::vector<MartItem>* PokeMap::getMartItems(u8 x, u8 y) {
    int key = x * static_cast<int>(mapInfo->cols) + y;
    auto it = martMap.find(key);
    return it==martMap.end()?nullptr:&it->second;
}

Warp* PokeMap::getNearbyWarp(int row, int col) {
    Warp *w = getWarp(row - 1, col);
    if (w == nullptr) {
        w = getWarp(row + 1, col);
        if (w == nullptr) {
            w = getWarp(row, col - 1);
            if (w == nullptr) {
                w = getWarp(row, col + 1);
            }
        }
    }
    return w;
}

void PokeMap::addEncounters(const std::vector<std::string> &a) {
    for (size_t i = 1; i < a.size(); i++) {
        std::vector<std::string> es = utils::split(a[i], ',');
        std::vector<Encounter> e;
        for (size_t j = 1; j < es.size(); j++) {
            e.emplace_back(es[j]);
        }
        encounters[es[0]] = e;
    }
}

Battler* PokeMap::getRandomEncounter(const std::string &type) {
    auto it = encounters.find(type);
    if (it == encounters.end()) {
        return nullptr;
    }
    std::vector<Encounter> &a = it->second;
    int r = utils::randInt(0, 100);
    int l = 0;
    for (const Encounter &e : a) {
        int n = l + e.chance;
        if (r < n) {
            return utils::rand01() < 0.5 ? new Battler(e.rLevel, Monster::MONSTER_MAP[e.rMonster->name])
                                         : new Battler(e.bLevel, Monster::MONSTER_MAP[e.bMonster->name]);
        }
        l = n;
    }
    throw std::runtime_error("Could not find encounter");
}

static void fillSight(PokeMap *pm, int v, int y, int x, int dy, int dx) {
    for (int i = 0; i < 4; i++) {
        y += dy;
        if (y < 0 || y >= static_cast<int>(pm->getGridRows())) {
            return;
        }
        x += dx;
        if (x < 0 || x >= static_cast<int>(pm->getGridCols())) {
            return;
        }
        int index = y * pm->getGridCols() + x;
        if (pm->sight[index] != 0) {
            return;
        }
        switch (pm->getTypeValue(y, x)) {
            case 0:
            case 3:
            case 5:
            case 10:
            case 11:
            case 19:
            case 20:
                return;
            default:
                pm->sight[index] = v;
        }
    }
}

void PokeMap::addTrainer(const std::string &s, const std::string &quote) {
    Trainer *t = new Trainer(s);
    if (t->y < 0 || t->x < 0 || t->y >= mapInfo->rows || t->x >= mapInfo->cols) {
        debugCheckpoint(("Trainer out of bounds in map " + name + " at (" +
                                 std::to_string(t->y) + "," + std::to_string(t->x) + ")").c_str());
    }
    int index = t->y * mapInfo->cols + t->x;
    trainers[index] = t;
    sight[index] = Trainer::tid;
    switch (t->dir) {
        case 1:
            fillSight(this, Trainer::tid, t->y, t->x, 1, 0);
            break;
        case 4:
            fillSight(this, Trainer::tid, t->y, t->x, -1, 0);
            break;
        case 6:
            fillSight(this, Trainer::tid, t->y, t->x, 0, -1);
            break;
        case 8:
            fillSight(this, Trainer::tid, t->y, t->x, 0, 1);
            break;
        default:
            throw std::runtime_error("BRO WHAT?!");
    }
    t->id = Trainer::tid++;
    t->setPhrases(quote);
}

void PokeMap::addTrainers(const std::vector<std::string> &a, const std::string &quote) {
    for (size_t i = 1; i < a.size(); i++) {
        addTrainer(a[i], quote);
    }
}

void PokeMap::addLeader(int i, const std::string &data, const std::string &tm, const std::string &quotes) {
    Trainer *t = new Trainer::Leader(i, data, tm);
    int index = t->y * mapInfo->cols + t->x;
    trainers[index] = t;
    sight[index] = -2;
    t->setPhrases(quotes);
}

void PokeMap::addGioRival(int i, const std::string &s, const std::string &quote) {
    Trainer *t = new Trainer::GioRival(i, s);
    if (t->y < 0 || t->x < 0 || t->y >= mapInfo->rows || t->x >= mapInfo->cols) {
        throw std::runtime_error("GioRival out of bounds in map " + name + " at (" +
                                 std::to_string(t->y) + "," + std::to_string(t->x) + ")");
    }
    int index = t->y * mapInfo->cols + t->x;
    trainers[index] = t;
    sight[index] = -2;
    t->setPhrases(quote);
}

void PokeMap::addE4Trainer(int id, const std::string &s, const std::string &p) {
    Trainer *t = new Trainer::E4Trainer(id, s);
    short index = t->y * mapInfo->cols + t->x;
    if(trainers[index])
    {
        delete(t);
        return;
    }
    trainers[index] = t;
    sight[index] = -2;
    t->setPhrases(p);
}

void PokeMap::addItem(const std::string &s) {
    WorldObject *wi = new WorldObject(s);
    int index = wi->y * mapInfo->cols + wi->x;
    wob[index] = wi;
}

void PokeMap::addItems(const std::vector<std::string> &a) {
    for (size_t i = 1; i < a.size(); i++) {
        addItem(a[i]);
    }
}

void PokeMap::addEncounter(const std::string &s) {
    WorldObject *we = new WorldObject::WorldEncounter(s);
    int index = we->y * mapInfo->cols + we->x;
    wob[index] = we;
}

void PokeMap::addSnorlaxEncounter(const std::string &s) {
    WorldObject *se = new WorldObject::SnorlaxEncounter(s);
    int index = se->y * mapInfo->cols + se->x;
    wob[index] = se;
}

void PokeMap::addGiver(const std::string &s, const std::string &q) {
    Giver *g = new Giver(s, q);
    int index = g->y * mapInfo->cols + g->x;
    npcs[index] = g;
}

void PokeMap::addGiver(const std::string &s, int n, const std::string &q) {
    Giver *g = new Giver::Aide(s, n, q);
    int index = g->y * mapInfo->cols + g->x;
    npcs[index] = g;
}

void PokeMap::addGiver(const std::string &s, const std::string &i, const std::string &q) {
    Giver *g = new Giver::IfGiver(s, i, q);
    int index = g->y * mapInfo->cols + g->x;
    npcs[index] = g;
}

void PokeMap::addMartItems(const std::vector<std::string> &a) {
    std::vector<MartItem> mi;
    for (size_t i = 2; i < a.size(); i++) {
        mi.emplace_back(a[i]);
    }
    std::vector<std::string> c = utils::split(a[1], ' ');
    int key = std::stoi(c[0]) * static_cast<int>(mapInfo->cols) + std::stoi(c[1]);
    martMap[key] = mi;
}

void PokeMap::addBlocker(const std::string &s, const std::string &q) {
    Blocker *b = new Blocker(s, q);
    int index = b->y * mapInfo->cols + b->x;
    npcs[index] = b;
}

void PokeMap::addBlocker(const std::string &s, const std::string &q, int n) {
    Blocker *b = new Blocker(s, q, n);
    int index = b->y * mapInfo->cols + b->x;
    npcs[index] = b;
}

void PokeMap::setHeal(int n) {
    healX = n;
    healY = n;
}

void PokeMap::addNpc(const std::string &s, const std::string &q) {
    Npc *n = new Npc(s, q);
    int index = n->y * mapInfo->cols + n->x;
    npcs[index] = n;
}

void PokeMap::addTrader(const std::string &s, const std::string &p) {
    Trader *t = new Trader(s, p);
    int index = t->y * mapInfo->cols + t->x;
    npcs[index] = t;
}

void PokeMap::stepOn(Player *p, int x, int y) {
    int index = y * mapInfo->cols + x;
    p->objectsCollected[wob[index]->id] = true;
    wob[index] = nullptr;
    if (x > 0) {
        int leftIndex = y * mapInfo->cols + (x - 1);
        if (wob[leftIndex] != nullptr) {
            p->objectsCollected[wob[leftIndex]->id] = true;
            wob[leftIndex] = nullptr;
        }
    }
    if (x < mapInfo->cols - 1) {
        int rightIndex = y * mapInfo->cols + (x + 1);
        if (wob[rightIndex] != nullptr) {
            p->objectsCollected[wob[rightIndex]->id] = true;
            wob[rightIndex] = nullptr;
        }
    }
}

std::string PokeMap::toString() const {
    return std::to_string(mapInfo->rows) + "," + std::to_string(mapInfo->cols);
}
