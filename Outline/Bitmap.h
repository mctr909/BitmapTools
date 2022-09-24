#ifndef __BITMAP_H__
#define __BITMAP_H__

#include "types.h"

#define DEFINE_SUPPORT_COLOR_256 (8)

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

    struct position {
        uint32 x;
        uint32 y;
    };

public:
    Bitmap(const string);
    Bitmap(int32, int32);
    ~Bitmap();
    void print_fileheader();
    void print_infoheader();
    int32 copy_data_overwrite(string, string);

public:
    infohead info_h;
    byte     *pPix;
    int32    error;

private:
    string   name;
    filehead file_h;
};

extern inline uint32 bitmap_stride(Bitmap const&);
extern inline uint32 bitmap_pix_index(Bitmap const&, const Bitmap::position);
extern inline uint32 bitmap_pix_ofs_index(Bitmap const&, const Bitmap::position, int32, int32);
extern inline void bitmap_pix_pos(Bitmap const&, Bitmap::position*, uint32);
#endif //__BITMAP_H__
