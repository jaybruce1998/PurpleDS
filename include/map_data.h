#ifndef MAP_DATA_H
#define MAP_DATA_H

#include <vector>
#include <string>
#include <map>
#include <nds.h>

struct MapInfo {
    short* data;  // Use 1D array
    u8 rows;
    u8 cols;
    
    MapInfo() : data(nullptr), rows(0), cols(0) {}
    MapInfo(short* d, u8 r, u8 c) : data(d), rows(r), cols(c) {}
};

// Function to initialize all map data
void initializeAllMaps();

// Helper function to dynamically allocate MapInfo structs
MapInfo* dynamicMap(short* templateData, u8 rows, u8 cols);

#endif // MAP_DATA_H
