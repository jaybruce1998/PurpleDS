#include "Warp.h"
#include "FlyLocation.h"
#include "Utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

Warp::Warp(PokeMap *pm, int row, int col) : pm(pm), row(row), col(col) {}

std::string Warp::toString() const {
    return pm->name + "," + std::to_string(row) + "," + std::to_string(col);
}

void Warp::buildWarps() {
    const size_t buffer_size = 256; // Buffer for warp data lines
    
    // Open file directly (nitrofs should be auto-initialized by build system)
    FILE* file = fopen("nitro:/Warp.txt", "r");
    if (!file) {
        return; // Failed to open file
    }
    
    // Allocate buffer on heap (not stack)
    char* buffer = (char*)malloc(buffer_size);
    if (!buffer) {
        fclose(file);
        return; // Failed to allocate memory
    }
    
    // Read all lines until EOF
    while (fgets(buffer, buffer_size, file)) {
        // Remove newline character if present
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        
        // Skip empty lines
        if (len == 0) continue;
        
        // Process warp line - split by comma
        std::vector<std::string> a = utils::split(buffer, ',');
        
        PokeMap *m1 = PokeMap::POKEMAPS[a[0]];
        PokeMap *m2 = PokeMap::POKEMAPS[a[3]];
        int sr = std::stoi(a[2]);
        int sc = std::stoi(a[1]);
        int er = std::stoi(a[5]);
        int ec = std::stoi(a[4]);
        m1->addWarp(sr, sc, new Warp(m2, er, ec));
        if (a[3].size() >= 10 && a[3].rfind("Pokecenter") == a[3].size() - 10) {
            FlyLocation::FLY_LOCATIONS[FlyLocation::normalize(a[3])] = new FlyLocation(m1, sc, sr + 1);
        }
        m2->addWarp(er, ec, new Warp(m1, sr, sc));
    }
    
    // Clean up
    free(buffer);
    fclose(file);
}
