#ifndef __BITMAP_H__
#define __BITMAP_H__

#include "types.h"

#define DEFINE_SUPPORT_COLOR_256 (8)
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
        uint32 width;
        uint32 height;
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

    struct position {
        uint32 x;
        uint32 y;
    };

public:
    Bitmap(const string);
    Bitmap(int32, int32, int32);
    ~Bitmap();
    void Save(const string);
    void SaveCopyData(string, string);
    void PrintFileHeader();
    void PrintInfoHeader();

public:
    infohead info_h;
    byte     *pPix;
    pix32    *pPalette;
    uint32   stride;
    int32    error;

private:
    string   name;
    filehead file_h;
    uint32   palette_size;

    uint32 get_stride();
};

extern inline uint32 bitmap_get_index(Bitmap const&, const Bitmap::position);
extern inline uint32 bitmap_get_index_ofs(Bitmap const&, const Bitmap::position, const int32, const int32);
extern inline void bitmap_get_pos(Bitmap const&, Bitmap::position*, uint32);
#endif //__BITMAP_H__
