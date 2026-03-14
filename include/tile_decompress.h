// Helper function to decompress 2-bit tile data
static inline unsigned char decompress_pixel(unsigned char packed_byte, int pixel_pos) {
    // pixel_pos: 0-3 (which 2-bit value to extract)
    switch (pixel_pos) {
        case 0: return (packed_byte >> 6) & 0x03;  // bits 7-6
        case 1: return (packed_byte >> 4) & 0x03;  // bits 5-4
        case 2: return (packed_byte >> 2) & 0x03;  // bits 3-2
        case 3: return packed_byte & 0x03;           // bits 1-0
        default: return 0;
    }
}

// Convert 2-bit value back to 8-bit grayscale
static inline unsigned char to_grayscale(unsigned char two_bit_value) {
    return two_bit_value * 85;  // 0, 85, 170, 255
}

// Get decompressed pixel value
static inline unsigned char get_pixel(const unsigned char tile[64], int x, int y) {
    int pixel_index = y * 16 + x;  // 16x16 tile
    int byte_index = pixel_index / 4;
    int pixel_pos = pixel_index % 4;
    
    unsigned char two_bit = decompress_pixel(tile[byte_index], pixel_pos);
    return to_grayscale(two_bit);
}
