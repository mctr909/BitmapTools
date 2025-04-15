#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>

using namespace std;

#include "Bitmap.h"

Bitmap::Bitmap(const string path) {
	ifstream fin(path, ios::in | ios::binary);
	if (!fin) {
		error = -1;
		return;
	}

	fin.read(reinterpret_cast<char *>(&header), sizeof(header));
	fin.read(reinterpret_cast<char *>(&info), sizeof(info));

	stride = ((info.width + 3) >> 2) << 2;
	info.imageSize = stride * info.height * static_cast<int32_t>(info.bits) >> 3;

	switch (info.bits) {
	case Type::COLOR8:
		paletteSize = sizeof(pix32) * 256;
		pPalette = reinterpret_cast<pix32 *>(calloc(1, paletteSize));
		if (nullptr == pPalette) {
			Dispose();
			error = -2;
			return;
		}
		fin.read(reinterpret_cast<char *>(pPalette), paletteSize);
		break;
	default:
		paletteSize = 0;
		pPalette = nullptr;
		break;
	}

	pPix = reinterpret_cast<uint8_t *>(malloc(info.imageSize));
	if (nullptr == pPix) {
		Dispose();
		error = -2;
		return;
	}
	pPixBackup = reinterpret_cast<uint8_t *>(malloc(info.imageSize));
	if (nullptr == pPixBackup) {
		Dispose();
		error = -2;
		return;
	}

	fin.seekg(header.offset, ios::beg);
	fin.read(reinterpret_cast<char *>(pPix), info.imageSize);
	fin.close();

	Backup();

	filePath = path;
	error = 0;
}

Bitmap::Bitmap(const int32_t width, const int32_t height, const Type type) {
	stride = ((width + 3) >> 2) << 2;

	info.chunkSize = sizeof(info);
	info.width = width;
	info.height = height;
	info.plane = 1;
	info.bits = type;
	info.compression = 0;
	info.imageSize = stride * height * static_cast<int32_t>(info.bits) >> 3;
	info.hResolution = 0;
	info.vResolution = 0;
	info.paletteColors = 0;
	info.importantIndex = 0;

	header.signature[0] = 'B';
	header.signature[1] = 'M';
	header.reserve1 = 0;
	header.reserve2 = 0;
	header.offset = (sizeof(header) + sizeof(info)) + paletteSize;
	header.size = header.offset + info.imageSize;

	pPix = reinterpret_cast<uint8_t *>(calloc(1, info.imageSize));
	if (nullptr == pPix) {
		Dispose();
		error = -2;
		return;
	}
	pPixBackup = reinterpret_cast<uint8_t *>(calloc(1, info.imageSize));
	if (nullptr == pPixBackup) {
		Dispose();
		error = -2;
		return;
	}

	switch (info.bits) {
	case Type::COLOR8:
		paletteSize = sizeof(pix32) * 256;
		pPalette = reinterpret_cast<pix32 *>(calloc(1, paletteSize));
		if (nullptr == pPalette) {
			Dispose();
			error = -2;
			return;
		}
		break;
	default:
		paletteSize = 0;
		pPalette = nullptr;
	}

	error = 0;
}

Bitmap::~Bitmap() {
	Dispose();
}

void
Bitmap::Dispose() {
	if (nullptr != pPalette) {
		free(pPalette);
	}
	if (nullptr != pPalette) {
		free(pPix);
	}
	if (nullptr != pPixBackup) {
		free(pPixBackup);
	}
}

void
Bitmap::Save(const string path) {
	ofstream fout(path, ios::out | ios::binary);
	if (!fout) {
		error = -1;
		return;
	}
	fout.write(reinterpret_cast<char *>(&header), sizeof(header));
	fout.write(reinterpret_cast<char *>(&info), sizeof(info));
	if (nullptr != pPalette) {
		fout.write(reinterpret_cast<char *>(pPalette), paletteSize);
	}
	if (nullptr != pPix) {
		fout.write(reinterpret_cast<char *>(pPix), info.imageSize);
	}
	fout.close();
}

void
Bitmap::PrintInfo() {
	cout << "<Information> " << endl;
	cout << "    Image Width      : " << info.width << " pixels" << endl;
	cout << "    Image Height     : " << info.height << " pixels" << endl;
	cout << "    Number of Planes : " << info.plane << endl;
	cout << "    Bits per Pixel   : " << static_cast<int32_t>(info.bits) << " bits/pixel" << endl;
	cout << "    Compression Type : " << info.compression << endl;
	cout << "    Image Size       : " << info.imageSize << " bytes" << endl;
	cout << "    H Resolution     : " << info.hResolution << " ppm" << endl;
	cout << "    V Resolution     : " << info.vResolution << " ppm" << endl;
	cout << "    Palette Colors   : " << info.paletteColors << endl;
	cout << "    Important Index  : " << info.importantIndex << endl;
	cout << endl;
}

void
Bitmap::Backup() {
	memcpy_s(pPixBackup, info.imageSize, pPix, info.imageSize);
}

void
Bitmap::Rollback() {
	memcpy_s(pPix, info.imageSize, pPixBackup, info.imageSize);
}
