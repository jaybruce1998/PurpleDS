// Auto-generated sprite data for DS ROM
#ifndef SPRITES_DATA_HPP
#define SPRITES_DATA_HPP

#include <stddef.h>
#include <map>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

// Real color palette (indexed)
const unsigned short spriteColors[24] = {
    0x8000, 0x8811, 0x8842, 0x8843, 0x8C31, 0x91FF, 0x9891, 0xA0F1, 0xA6C4, 0xCE73, 0xD134, 0xD154, 0xD174, 0xD1B4, 0xDAD6, 0xE527,
    0xF680, 0xF681, 0xF6C5, 0xF6EA, 0xF773, 0xF7BD, 0xFBE8, 0xFFBF,
};

// External declarations - all sprites are 16x16 (256 pixels)
extern std::map<std::string, const unsigned char*> SPRITE_DATA;
void initializeSprites();

// RED and SEEL sprite frame arrays (non-static for Gui access)
extern const unsigned char RED_0_data[256];
extern const unsigned char RED_1_data[256];
extern const unsigned char RED_2_data[256];
extern const unsigned char RED_3_data[256];
extern const unsigned char RED_4_data[256];
extern const unsigned char RED_5_data[256];
extern const unsigned char RED_6_data[256];
extern const unsigned char RED_7_data[256];
extern const unsigned char RED_8_data[256];
extern const unsigned char RED_9_data[256];

extern const unsigned char SEEL_0_data[256];
extern const unsigned char SEEL_1_data[256];
extern const unsigned char SEEL_2_data[256];
extern const unsigned char SEEL_3_data[256];
extern const unsigned char SEEL_4_data[256];
extern const unsigned char SEEL_5_data[256];
extern const unsigned char SEEL_6_data[256];
extern const unsigned char SEEL_7_data[256];
extern const unsigned char SEEL_8_data[256];
extern const unsigned char SEEL_9_data[256];

#ifdef __cplusplus
}
#endif

#endif // SPRITES_DATA_HPP
