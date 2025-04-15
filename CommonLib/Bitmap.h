#ifndef __BITMAP_H__
#define __BITMAP_H__

#include "types.h"

class Bitmap {
public:
	enum struct Type : uint16_t {
		COLOR8 = 8,
		COLOR24 = 24
	};
#pragma pack(push, 4)
	struct INFO {
		uint32_t chunkSize;
		int32_t  width;
		int32_t  height;
		uint16_t plane;
		Type     bits;
		uint32_t compression;
		uint32_t imageSize;
		uint32_t hResolution;
		uint32_t vResolution;
		uint32_t paletteColors;
		uint32_t importantIndex;
	};
#pragma pack(pop)
#pragma pack(push, 1)
	struct pix24 {
		uint8_t b;
		uint8_t g;
		uint8_t r;
	};
#pragma pack(pop)
#pragma pack(push, 4)
	struct pix32 {
		uint8_t b;
		uint8_t g;
		uint8_t r;
		uint8_t a;
	};
#pragma pack(pop)

private:
#pragma pack(push, 2)
	struct FILE_HEAD {
		uint8_t  signature[2];
		uint32_t size;
		uint16_t reserve1;
		uint16_t reserve2;
		uint32_t offset;
	};
#pragma pack(pop)

public:
	string  filePath = "";
	INFO    info = {};
	int32_t stride = 0;
	pix32   *pPalette = nullptr;
	uint8_t *pPix = nullptr;
	int32_t error = 0;

private:
	FILE_HEAD header = {};
	uint32_t  paletteSize = 0;
	uint8_t   *pPixBackup = nullptr;

public:
	/* ビットマップファイルを読込みます */
	Bitmap(const string path);
	/* ビットマップを作成します */
	Bitmap(const int32_t width, const int32_t height, const Type type);
	~Bitmap();

private:
	Bitmap() { }
	void Dispose();

public:
	/* ビットマップファイルとして保存します */
	void Save(const string path);
	/* ピクセル情報を表示します */
	void PrintInfo();
	/* 編集中のピクセルバッファをバックアップします */
	void Backup();
	/* 編集中のピクセルバッファをバックアップ時の状態に戻します */
	void Rollback();

public:
	/* 指定された位置(pos)に対するピクセルバッファのインデックスを取得します */
	static uint32_t GetIndex(Bitmap const &bmp, point const &pos);
	/* 指定された位置(pos)に対するピクセルバッファのインデックスを取得します */
	static uint32_t GetIndex(Bitmap const &bmp, point const &pos, const int32_t dx, const int32_t dy);
	/* 指定されたピクセルバッファのインデックス(index)に対する位置(resultPos)を取得します */
	static void GetPos(Bitmap const &bmp, const uint32_t index, point *resultPos);
	/* 指定されたピクセル(p)の輝度を取得します */
	static double GetLum(pix24 const &p);
	/* 指定されたピクセル(p)の輝度を取得します */
	static double GetLum(pix32 const &p);
};

inline uint32_t
Bitmap::GetIndex(Bitmap const &bmp, point const &pos) {
	int32_t x = pos.x;
	int32_t y = pos.y;
	int32_t isValid = x < bmp.info.width;
	isValid &= y < bmp.info.height;
	x += y * bmp.stride;
	x *= isValid;
	x += isValid - 1;
	return x;
}

inline uint32_t
Bitmap::GetIndex(Bitmap const &bmp, point const &pos, const int32_t dx, const int32_t dy) {
	int32_t x = dx + pos.x;
	int32_t y = dy + pos.y;
	int32_t isValid = x >= 0;
	isValid &= y >= 0;
	isValid &= x < bmp.info.width;
	isValid &= y < bmp.info.height;
	x += y * bmp.stride;
	x *= isValid;
	x += isValid - 1;
	return x;
}

inline void
Bitmap::GetPos(Bitmap const &bmp, const uint32_t index, point *resultPos) {
	resultPos->x = index % bmp.stride;
	resultPos->y = index / bmp.stride;
}

inline double
Bitmap::GetLum(pix24 const &p) {
	return 2.831e-04 * p.b + 2.804e-03 * p.g + 8.337e-04 * p.r;
}

inline double
Bitmap::GetLum(pix32 const &p) {
	return 2.831e-04 * p.b + 2.804e-03 * p.g + 8.337e-04 * p.r;
}
#endif //__BITMAP_H__
