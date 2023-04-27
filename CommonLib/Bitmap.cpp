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

    stride = ((info_h.width + 3) >> 2) << 2;
    pixel_count = stride * info_h.height;

    switch (info_h.bits) {
    case BITMAP_COLOR_8BIT:
        pPalette = reinterpret_cast<pix32*>(calloc(256, sizeof(pix32)));
        if (NULL == pPalette) {
            error = -2;
            return;
        }
        palette_size = sizeof(pix32) * 256;
        fin.read(reinterpret_cast<char*>(pPalette), palette_size);
        break;
    default:
        pPalette = NULL;
        palette_size = 0;
        break;
    }

    pPixWork = reinterpret_cast<byte*>(calloc(info_h.imagesize, 1));
    if (NULL == pPixWork) {
        free(pPalette);
        error = -2;
        return;
    }

    fin.seekg(file_h.offset, ios::beg);
    fin.read(reinterpret_cast<char*>(pPixWork), info_h.imagesize);
    fin.seekg(current_pos);
    fin.close();

    name = path;
    error = 0;
}

Bitmap::Bitmap(int32 width, int32 height, int32 bits) {
    stride = ((width + 3) >> 2) << 2;
    pixel_count = stride * height;

    info_h.headsize = sizeof(info_h);
    info_h.width = width;
    info_h.height = height;
    info_h.plane = 1;
    info_h.bits = bits;
    info_h.compression = 0;
    info_h.imagesize = pixel_count * bits >> 3;
    info_h.h_resolution = 0;
    info_h.v_resolution = 0;
    info_h.color_id = 0;
    info_h.important_id = 0;

    file_h.type[0] = 'B';
    file_h.type[1] = 'M';
    file_h.reserve1 = 0;
    file_h.reserve2 = 0;
    file_h.offset = (sizeof(file_h) + sizeof(info_h)) + palette_size;
    file_h.size = file_h.offset + info_h.imagesize;

    pPixWork = reinterpret_cast<byte*>(calloc(1, info_h.imagesize));
    if (NULL == pPixWork) {
        error = -2;
        return;
    }

    switch (bits) {
    case BITMAP_COLOR_8BIT:
        pPalette = reinterpret_cast<pix32*>(calloc(256, sizeof(pix32)));
        if (NULL == pPalette) {
            free(pPixWork);
            error = -2;
            return;
        }
        palette_size = sizeof(pix32) * 256;
        break;
    default:
        pPalette = NULL;
        palette_size = 0;
    }

    error = 0;
}

Bitmap::~Bitmap() {
    free(pPalette);
    free(pPixWork);
    free(pPixBackup);
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
    if (NULL != pPixWork) {
        fout.write(reinterpret_cast<char*>(pPixWork), info_h.imagesize);
    }
    fout.close();
}

void
Bitmap::PrintHeader() {
    cout << "<File Header> " << endl;
    cout << "File Type        : " << file_h.type[0] << file_h.type[1] << endl;
    cout << "File Size        : " << file_h.size << " bytes" << endl;
    cout << "Image Data Offset: " << file_h.offset << " bytes" << endl;
    cout << endl;
    cout << "<Information Header> " << endl;
    cout << "Header Size      : " << info_h.headsize << " bytes" << endl;
    cout << "Image Width      : " << info_h.width << " pixels" << endl;
    cout << "Image Height     : " << info_h.height << " pixels" << endl;
    cout << "Number of Planes : " << info_h.plane << endl;
    cout << "Bits per Pixel   : " << info_h.bits << " bits/pixel" << endl;
    cout << "Compression Type : " << info_h.compression << endl;
    cout << "Image Size       : " << info_h.imagesize << " bytes" << endl;
    cout << "H Resolution     : " << info_h.h_resolution << " ppm" << endl;
    cout << "V Resolution     : " << info_h.v_resolution << " ppm" << endl;
    cout << "Color Index      : " << info_h.color_id << endl;
    cout << "Important Index  : " << info_h.important_id << endl;
    cout << endl;
}

void
Bitmap::Backup() {
    if (NULL == pPixBackup) {
        pPixBackup = reinterpret_cast<byte*>(malloc(info_h.imagesize));
    }
    memcpy_s(pPixBackup, info_h.imagesize, pPixWork, info_h.imagesize);
}

void
Bitmap::Rollback() {
    if (NULL != pPixBackup) {
        memcpy_s(pPixWork, info_h.imagesize, pPixBackup, info_h.imagesize);
    }
}