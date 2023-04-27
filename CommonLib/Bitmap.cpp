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

    fin.read(reinterpret_cast<char*>(&m_header), sizeof(m_header));
    fin.read(reinterpret_cast<char*>(&m_info), sizeof(m_info));

    m_stride = ((m_info.width + 3) >> 2) << 2;

    switch (m_info.bits) {
    case BITMAP_COLOR_8BIT:
        mp_palette = reinterpret_cast<pix32*>(calloc(256, sizeof(pix32)));
        if (NULL == mp_palette) {
            error = -2;
            return;
        }
        m_palette_size = sizeof(pix32) * 256;
        fin.read(reinterpret_cast<char*>(mp_palette), m_palette_size);
        break;
    default:
        mp_palette = NULL;
        m_palette_size = 0;
        break;
    }

    mp_pix = reinterpret_cast<byte*>(calloc(m_info.imagesize, 1));
    if (NULL == mp_pix) {
        free(mp_palette);
        error = -2;
        return;
    }

    fin.seekg(m_header.offset, ios::beg);
    fin.read(reinterpret_cast<char*>(mp_pix), m_info.imagesize);
    fin.seekg(current_pos);
    fin.close();

    m_name = path;
    error = 0;
}

Bitmap::Bitmap(int32 width, int32 height, int32 bits) {
    m_stride = ((width + 3) >> 2) << 2;

    m_info.headsize = sizeof(m_info);
    m_info.width = width;
    m_info.height = height;
    m_info.plane = 1;
    m_info.bits = bits;
    m_info.compression = 0;
    m_info.imagesize = m_stride * height * bits >> 3;
    m_info.h_resolution = 0;
    m_info.v_resolution = 0;
    m_info.color_id = 0;
    m_info.important_id = 0;

    m_header.type[0] = 'B';
    m_header.type[1] = 'M';
    m_header.reserve1 = 0;
    m_header.reserve2 = 0;
    m_header.offset = (sizeof(m_header) + sizeof(m_info)) + m_palette_size;
    m_header.size = m_header.offset + m_info.imagesize;

    mp_pix = reinterpret_cast<byte*>(calloc(1, m_info.imagesize));
    if (NULL == mp_pix) {
        error = -2;
        return;
    }

    switch (bits) {
    case BITMAP_COLOR_8BIT:
        mp_palette = reinterpret_cast<pix32*>(calloc(256, sizeof(pix32)));
        if (NULL == mp_palette) {
            free(mp_pix);
            error = -2;
            return;
        }
        m_palette_size = sizeof(pix32) * 256;
        break;
    default:
        mp_palette = NULL;
        m_palette_size = 0;
    }

    error = 0;
}

Bitmap::~Bitmap() {
    free(mp_palette);
    free(mp_pix);
    free(mp_pix_backup);
}

void
Bitmap::Save(const string path) {
    ofstream fout(path, ios::out | ios::binary);
    if (!fout) {
        error = -1;
        return;
    }
    fout.write(reinterpret_cast<char*>(&m_header), sizeof(m_header));
    fout.write(reinterpret_cast<char*>(&m_info), sizeof(m_info));
    if (NULL != mp_palette) {
        fout.write(reinterpret_cast<char*>(mp_palette), m_palette_size);
    }
    if (NULL != mp_pix) {
        fout.write(reinterpret_cast<char*>(mp_pix), m_info.imagesize);
    }
    fout.close();
}

void
Bitmap::PrintHeader() {
    cout << "<File Header> " << endl;
    cout << "File Type        : " << m_header.type[0] << m_header.type[1] << endl;
    cout << "File Size        : " << m_header.size << " bytes" << endl;
    cout << "Image Data Offset: " << m_header.offset << " bytes" << endl;
    cout << endl;
    cout << "<Information Header> " << endl;
    cout << "Header Size      : " << m_info.headsize << " bytes" << endl;
    cout << "Image Width      : " << m_info.width << " pixels" << endl;
    cout << "Image Height     : " << m_info.height << " pixels" << endl;
    cout << "Number of Planes : " << m_info.plane << endl;
    cout << "Bits per Pixel   : " << m_info.bits << " bits/pixel" << endl;
    cout << "Compression Type : " << m_info.compression << endl;
    cout << "Image Size       : " << m_info.imagesize << " bytes" << endl;
    cout << "H Resolution     : " << m_info.h_resolution << " ppm" << endl;
    cout << "V Resolution     : " << m_info.v_resolution << " ppm" << endl;
    cout << "Color Index      : " << m_info.color_id << endl;
    cout << "Important Index  : " << m_info.important_id << endl;
    cout << endl;
}

void
Bitmap::Backup() {
    if (NULL == mp_pix_backup) {
        mp_pix_backup = reinterpret_cast<byte*>(malloc(m_info.imagesize));
    }
    memcpy_s(mp_pix_backup, m_info.imagesize, mp_pix, m_info.imagesize);
}

void
Bitmap::Rollback() {
    if (NULL != mp_pix_backup) {
        memcpy_s(mp_pix, m_info.imagesize, mp_pix_backup, m_info.imagesize);
    }
}