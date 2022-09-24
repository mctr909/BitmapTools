#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>

using namespace std;

#include "Bitmap.h"

Bitmap::Bitmap(const string bmp_file) {
    streampos current_pos;

    ifstream fin(bmp_file, ios::in | ios::binary);
    if (!fin) {
        error = -1;
        return;
    }

    fin.read(reinterpret_cast<char*>(&file_h), sizeof(file_h));
    fin.read(reinterpret_cast<char*>(&info_h), sizeof(info_h));

    fin.seekg(file_h.offset, ios::beg);
    pPix = (byte*)calloc(info_h.imagesize, 1);
    fin.read(reinterpret_cast<char*>(pPix), info_h.imagesize);

    fin.seekg(current_pos);
    fin.close();

    name = bmp_file;
    error = 0;
}

Bitmap::~Bitmap() {
    if (NULL != pPix) {
        free(pPix);
    }
}

void
Bitmap::print_fileheader() {
    cout << "<File Header> " << endl;
    cout << "File Type        : " << file_h.type[0] << file_h.type[1] << endl;
    cout << "File Size        : " << file_h.size << " bytes" << endl;
    cout << "Image Data Offset: " << file_h.offset << " bytes" << endl;
    cout << endl;
}

void
Bitmap::print_infoheader() {
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

int32
Bitmap::copy_data_overwrite(string inName, string outName) {
    ifstream fin(inName, ios::in | ios::binary);
    if (!fin) {
        return (-1);
    }

    fin.seekg(0, std::ios::end);
    auto size = static_cast<unsigned long>(fin.tellg());
    fin.seekg(0, std::ios::beg);

    auto pdata = new byte[size];
    fin.read(reinterpret_cast<char*>(pdata), size);
    fin.close();

    for (uint32 i = 0, j = file_h.offset; i < info_h.imagesize; i++, j++) {
        pdata[j] = pPix[i];
    }

    ofstream fout(outName, ios::out | ios::binary);
    if (!fout)
    {
        return (-2);
    }

    fout.write(reinterpret_cast<char*>(pdata), size);
    fout.close();

    return (0);
}

inline uint32
bitmap_stride(Bitmap const& bmp) {
    return (((bmp.info_h.width + 3) >> 2) << 2);
}

inline uint32
bitmap_pix_index(Bitmap const& bmp, const Bitmap::position p) {
    if ((p.x >= bmp.info_h.width) || (p.y >= bmp.info_h.height)) {
        return ULONG_MAX;
    }
    auto stride = bitmap_stride(bmp);
    return ((p.x + (stride * p.y)) * bmp.info_h.pixel >> 3);
}

inline uint32
bitmap_pix_ofs_index(Bitmap const& bmp, const Bitmap::position p, int32 dx, int32 dy) {
    auto x = static_cast<int32>(p.x) + dx;
    auto y = static_cast<int32>(p.y) + dy;
    if ((x < 0) || (x >= bmp.info_h.width) || (y < 0) || (y >= bmp.info_h.height)) {
        return ULONG_MAX;
    }
    auto stride = bitmap_stride(bmp);
    return ((x + (stride * y)) * bmp.info_h.pixel >> 3);
}

inline void
bitmap_pix_pos(Bitmap const& bmp, Bitmap::position* pos, uint32 index) {
    auto stride = bitmap_stride(bmp);
    pos->x = (index % stride) / (bmp.info_h.pixel >> 3);
    pos->y = index / stride;
}
