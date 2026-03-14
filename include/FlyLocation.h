#pragma once

#include "Utils.h"

// Forward declarations for SDL types
struct SDL_Renderer;
struct SDL_Texture;

class PokeMap;

class FlyLocation {
public:
    static const char* INDEX_MEANINGS[39];
    static const char* NAME_MAP[11][11];
    static std::unordered_map<std::string, FlyLocation*> FLY_LOCATIONS;

    static void buildWorldMap();
    static bool isRed(int i, int j);
    static void visit(const std::string &name);
    static std::string normalize(const std::string &s);
    static bool loadMapTexture(SDL_Renderer *renderer, const std::string &path);

    bool visited = false;
    PokeMap *dest = nullptr;
    int x = 0;
    int y = 0;

    FlyLocation() = default;
    FlyLocation(PokeMap *dest, int x, int y);
};
