#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>

using namespace std;

#include "Bitmap.h"

Bitmap::Bitmap(const string path) {
    streampos current_pos;

    ifstream fin(path, ios::in | ios::binary);
    if (!fin) {
        error = -1;
        return;
    }

    fin.read(reinterpret_cast<char*>(&file_h), sizeof(file_h));
    fin.read(reinterpret_cast<char*>(&info_h), sizeof(info_h));

    fin.seekg(file_h.offset, ios::beg);
    pPix = reinterpret_cast<byte*>(calloc(info_h.imagesize, 1));
    fin.read(reinterpret_cast<char*>(pPix), info_h.imagesize);

    fin.seekg(current_pos);
    fin.close();

    stride = get_stride();
    name = path;
    error = 0;
}

Bitmap::Bitmap(int32 width, int32 height, int32 bits) {
    switch (bits) {
    case 8:
        pPix = reinterpret_cast<byte*>(calloc(width * height, 1));
        pPalette = reinterpret_cast<pix32*>(calloc(256, sizeof(pix32)));
        palette_size = sizeof(pix32) * 256;
        break;
    case 24:
        pPix = reinterpret_cast<byte*>(calloc(width * height, sizeof(pix24)));
        pPalette = NULL;
        palette_size = 0;
        break;
    default:
        pPix = NULL;
        pPalette = NULL;
        palette_size = 0;
    }

    info_h.headsize = sizeof(info_h);
    info_h.width = width;
    info_h.height = height;
    info_h.plane = 1;
    info_h.pixel = bits;
    info_h.compression = 0;
    info_h.imagesize = width * height * bits >> 3;
    info_h.h_resolution = 0;
    info_h.v_resolution = 0;
    info_h.color_id = 0;
    info_h.important_id = 0;

    file_h.type[0] = 'B';
    file_h.type[1] = 'M';
    file_h.size = (file_h.offset - 6) + palette_size + info_h.imagesize;
    file_h.reserve1 = 0;
    file_h.reserve2 = 0;
    file_h.offset = (sizeof(file_h) + sizeof(info_h)) + palette_size;

    stride = get_stride();
    error = 0;
}

Bitmap::~Bitmap() {
    if (NULL != pPix) {
        free(pPix);
    }
    if (NULL != pPalette) {
        free(pPalette);
    }
}

void
Bitmap::Save(const string path) {
    ofstream fout(path, ios::out | ios::binary);
    if (!fout) {
        error = -1;
        return;
    }

    fout.write(reinterpret_cast<char*>(&file_h), sizeof(file_h));
    fout.write(reinterpret_cast<char*>(&info_h), sizeof(info_h));
    if (NULL != pPalette) {
        fout.write(reinterpret_cast<char*>(pPalette), palette_size);
    }
    if (NULL != pPix) {
        fout.write(reinterpret_cast<char*>(pPix), info_h.imagesize);
    }
    fout.close();
}

void
Bitmap::SaveCopyData(string inName, string outName) {
    ifstream fin(inName, ios::in | ios::binary);
    if (!fin) {
        error = -1;
        return;
    }

    fin.seekg(0, std::ios::end);
    auto size = static_cast<uint32>(fin.tellg());
    fin.seekg(0, std::ios::beg);

    auto pdata = new byte[size];
    fin.read(reinterpret_cast<char*>(pdata), size);
    fin.close();

    for (uint32 i = 0, j = file_h.offset; i < info_h.imagesize; i++, j++) {
        pdata[j] = pPix[i];
    }

    ofstream fout(outName, ios::out | ios::binary);
    if (!fout) {
        error = -2;
        return;
    }

    fout.write(reinterpret_cast<char*>(pdata), size);
    fout.close();
}

void
Bitmap::PrintFileHeader() {
    cout << "<File Header> " << endl;
    cout << "File Type        : " << file_h.type[0] << file_h.type[1] << endl;
    cout << "File Size        : " << file_h.size << " bytes" << endl;
    cout << "Image Data Offset: " << file_h.offset << " bytes" << endl;
    cout << endl;
}

void
Bitmap::PrintInfoHeader() {
    cout << "<Information Header> " << endl;
    cout << "Header Size      : " << info_h.headsize << " bytes" << endl;
    cout << "Image Width      : " << info_h.width << " pixels" << endl;
    cout << "Image Height     : " << info_h.height << " pixels" << endl;
    cout << "Number of Planes : " << info_h.plane << endl;
    cout << "Bits per Pixel   : " << info_h.pixel << " bits/pixel" << endl;
    cout << "Compression Type : " << info_h.compression << endl;
    cout << "Image Size       : " << info_h.imagesize << " bytes" << endl;
    cout << "H Resolution     : " << info_h.h_resolution << " ppm" << endl;
    cout << "V Resolution     : " << info_h.v_resolution << " ppm" << endl;
    cout << "Color Index      : " << info_h.color_id << endl;
    cout << "Important Index  : " << info_h.important_id << endl;
    cout << endl;
}

uint32
Bitmap::get_stride() {
    return (((info_h.width + 3) >> 2) << 2);
}

inline uint32
bitmap_get_index(Bitmap const& bmp, const Bitmap::position pos) {
    if ((pos.x >= bmp.info_h.width) || (pos.y >= bmp.info_h.height)) {
        return UINT32_MAX;
    }
    return ((pos.x + (bmp.stride * pos.y)));
}

inline uint32
bitmap_get_index_ofs(Bitmap const& bmp, const Bitmap::position pos, const int32 dx, const int32 dy) {
    auto x = static_cast<int32>(pos.x) + dx;
    auto y = static_cast<int32>(pos.y) + dy;
    if ((x < 0) || (x >= bmp.info_h.width) || (y < 0) || (y >= bmp.info_h.height)) {
        return UINT32_MAX;
    }
    return (x + (bmp.stride * y));
}

inline void
bitmap_get_pos(Bitmap const& bmp, Bitmap::position* pos, uint32 index) {
    pos->x = index % bmp.stride;
    pos->y = index / bmp.stride;
}
