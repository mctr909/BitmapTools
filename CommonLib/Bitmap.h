#ifndef __BITMAP_H__
#define __BITMAP_H__

#include "types.h"

#define BITMAP_COLOR_8BIT  (8)
#define BITMAP_COLOR_24BIT (24)

class Bitmap {
public:
#pragma pack(push, 2)
    struct FileHead {
        char   type[2];
        uint32 size;
        uint16 reserve1;
        uint16 reserve2;
        uint32 offset;
    };
#pragma pack(pop)

#pragma pack(push, 4)
    struct Info {
        uint32 headsize;
        int32 width;
        int32 height;
        uint16 plane;
        uint16 bits;
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
    Info     m_info = { 0 };
    int32    m_stride = 0;
    pix32*   mp_palette = NULL;
    byte*    mp_pix = NULL;
    int32    error = 0;

private:
    FileHead m_header = { 0 };
    uint32   m_palette_size = 0;
    string   m_name = "";
    byte*    mp_pix_backup = NULL;

public:
    Bitmap(const string path);
    Bitmap(int32 width, int32 height, int32 bits);
    ~Bitmap();

public:
    void Save(const string path);
    void PrintHeader();
    void Backup();
    void Rollback();
};

inline uint32
bitmap_get_index(Bitmap const& bmp, const point pos) {
    if ((pos.x >= bmp.m_info.width) || (pos.y >= bmp.m_info.height)) {
        return INVALID_INDEX;
    }
    return ((pos.x + (bmp.m_stride * pos.y)));
}

inline uint32
bitmap_get_index_ofs(Bitmap const& bmp, const point pos, const int32 dx, const int32 dy) {
    auto x = pos.x + dx;
    auto y = pos.y + dy;
    if ((x < 0) || (x >= bmp.m_info.width) || (y < 0) || (y >= bmp.m_info.height)) {
        return INVALID_INDEX;
    }
    return (x + (bmp.m_stride * y));
}

inline void
bitmap_get_pos(Bitmap const& bmp, point* pos, const uint32 index) {
    pos->x = index % bmp.m_stride;
    pos->y = index / bmp.m_stride;
}

inline double
bitmap_get_lum(byte r, byte g, byte b) {
    return (0.299 * r + 0.587 * g + 0.114 * b) / 255.0;
}

#endif //__BITMAP_H__
