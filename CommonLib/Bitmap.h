#ifndef __BITMAP_H__
#define __BITMAP_H__

#include "types.h"

#define DEFINE_SUPPORT_COLOR_256   (8)
#define DEFINE_SUPPORT_COLOR_24BIT (24)

class Bitmap {
public:
#pragma pack(push, 2)
    struct filehead {
        char   type[2];
        uint32 size;
        uint16 reserve1;
        uint16 reserve2;
        uint32 offset;
    };
#pragma pack(pop)

#pragma pack(push, 4)
    struct infohead {
        uint32 headsize;
        int32 width;
        int32 height;
        uint16 plane;
        uint16 pixel;
        uint32 compression;
        uint32 imagesize;
        uint32 h_resolution;
        uint32 v_resolution;
        uint32 color_id;
        uint32 important_id;
    };
#pragma pack(pop)

#pragma pack(push, 1)
    struct pix24 {
        byte b;
        byte g;
        byte r;
    };
#pragma pack(pop)

#pragma pack(push, 4)
    struct pix32 {
        byte b;
        byte g;
        byte r;
        byte a;
    };
#pragma pack(pop)

public:
    Bitmap(const string);
    Bitmap(int32, int32, int32);
    ~Bitmap();
    void Save(const string);
    void PrintFileHeader();
    void PrintInfoHeader();

public:
    infohead info_h;
    byte     *pPix;
    pix32    *pPalette;
    int32    stride;
    int32    error;

private:
    string   name;
    filehead file_h;
    uint32   palette_size;
};

inline uint32
bitmap_get_index(Bitmap const& bmp, const point pos) {
    if ((pos.x >= bmp.info_h.width) || (pos.y >= bmp.info_h.height)) {
        return UINT32_MAX;
    }
    return ((pos.x + (bmp.stride * pos.y)));
}

inline uint32
bitmap_get_index_ofs(Bitmap const& bmp, const point pos, const int32 dx, const int32 dy) {
    auto x = pos.x + dx;
    auto y = pos.y + dy;
    if ((x < 0) || (x >= bmp.info_h.width) || (y < 0) || (y >= bmp.info_h.height)) {
        return UINT32_MAX;
    }
    return (x + (bmp.stride * y));
}

inline void
bitmap_get_pos(Bitmap const& bmp, point* pos, const uint32 index) {
    pos->x = index % bmp.stride;
    pos->y = index / bmp.stride;
}

#endif //__BITMAP_H__
