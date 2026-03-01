// Auto-generated font data for DS ROM
#ifndef FONT_DATA_HPP
#define FONT_DATA_HPP

#include <stddef.h>
#include <map>
#include <cstdint>
#include <nds.h>

#ifdef __cplusplus
extern "C" {
#endif

// Font character data - maps characters to 8x8 bitmasks
// Special values: 0 = male symbol, 1 = female symbol
extern std::map<int, unsigned long long> FONT_DATA;
void initializeFont();
void drawText(const char* text, int startX, int startY, u16 color, u16* buffer);

#ifdef __cplusplus
}
#endif

#endif // FONT_DATA_HPP
