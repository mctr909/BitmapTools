#ifndef __BITMAP_H__
#define __BITMAP_H__

#include "types.h"

class Bitmap {
public:
	/**
	 * ピクセル形式
	 */
	enum struct Type : uint16_t {
		// 2色インデックスカラー
		COLOR2 = 1,
		// 4色インデックスカラー
		COLOR4 = 2,
		// 16色インデックスカラー
		COLOR16 = 4,
		// 256色インデックスカラー
		COLOR256 = 8,
		// 24bitRGB
		COLOR24BIT = 24
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
	pix32   *pPalette = nullptr;
	uint8_t *pPix = nullptr;

private:
	FILE_HEAD header = {};
	uint32_t  paletteSize = 0;
	int32_t   stride = 0;
	int32_t   shiftX = 0;
	int32_t   maskX = 0;
	int32_t   maskVal = 255;
	uint8_t   *pPixBackup = nullptr;

public:
	/**
	 * ビットマップファイルを読込みます
	 * @param[in] path ビットマップファイルのパス
	 */
	Bitmap(const string path);
	/**
	 * ビットマップを作成します
	 * @param[in] width 幅
	 * @param[in] height 高さ
	 * @param[in] type
	 */
	Bitmap(const int32_t width, const int32_t height, const Type type);
	~Bitmap();

private:
	Bitmap() { }
	void Dispose();

public:
	/**
	 * ビットマップファイルとして保存します
	 * @param[in] path 保存するビットマップファイルのパス
	 * @returns 保存結果(true:成功、false:失敗)
	 */
	bool Save(const string path);
	/**
	 * ピクセル情報を表示します
	 */
	void PrintInfo() const;
	/**
	 * 編集中のピクセルバッファをバックアップします
	 */
	void Backup();
	/**
	 * 編集中のピクセルバッファをバックアップ時の状態に戻します
	 */
	void Rollback();

public:
	/**
	 * 指定された座標に対するピクセルバッファのインデックスを取得します
	 * @param[in] pos 座標
	 * @returns ピクセルバッファのインデックス
	 */
	uint32_t GetIndex(point const &pos) const;
	/**
	 * 指定された座標とその差分に対するピクセルバッファのインデックスを取得します
	 * @param[in] pos 座標
	 * @param[in] dx x軸方向差分
	 * @param[in] dy y軸方向差分
	 * @returns ピクセルバッファのインデックス
	 */
	uint32_t GetIndex(point const &pos, const int32_t dx, const int32_t dy) const;
	/**
	 * 指定された座標のピクセル色を取得します
	 * @param[in] pos 座標
	 * @returns ピクセル色
	 */
	pix32 GetIndexedColor(point const &pos) const;
	/**
	 * 指定された座標に色インデックスを設定します
	 * @param[in] pos 座標
	 * @param[in] colorIndex 色インデックス
	 */
	void SetIndexedColor(point const &pos, int32_t const &colorIndex) const;

public:
	/**
	 * 指定されたピクセルの輝度を取得します
	 * @param[in] p ピクセル
	 * @returns 輝度
	 */
	static double GetLum(pix24 const &p);
	/**
	 * 指定されたピクセルの輝度を取得します
	 * @param[in] p ピクセル
	 * @returns 輝度
	 */
	static double GetLum(pix32 const &p);
};

inline uint32_t
Bitmap::GetIndex(point const &pos) const {
	int32_t x = pos.x;
	int32_t y = pos.y;
	int32_t isValid = x < info.width;
	isValid &= y < info.height;
	x >>= shiftX;
	x += y * stride;
	x *= isValid;
	x += isValid - 1;
	return x;
}

inline uint32_t
Bitmap::GetIndex(point const &pos, const int32_t dx, const int32_t dy) const {
	int32_t x = dx + pos.x;
	int32_t y = dy + pos.y;
	int32_t isValid = x >= 0;
	isValid &= y >= 0;
	isValid &= x < info.width;
	isValid &= y < info.height;
	x >>= shiftX;
	x += y * stride;
	x *= isValid;
	x += isValid - 1;
	return x;
}

inline Bitmap::pix32
Bitmap::GetIndexedColor(point const &pos) const {
	int32_t x = pos.x;
	int32_t shift = x & maskX;
	int32_t shiftUnit = static_cast<int32_t>(info.bits);
	shift *= shiftUnit;
	shift = 8 - shift;
	shift -= shiftUnit;
	x >>= shiftX;
	x += pos.y * stride;
	x = pPix[x] >> shift;
	x &= maskVal;
	return pPalette[x];
}

inline void
Bitmap::SetIndexedColor(point const &pos, int32_t const &colorIndex) const {
	int32_t x = pos.x;
	int32_t shift = x & maskX;
	int32_t shiftUnit = static_cast<int32_t>(info.bits);
	shift *= shiftUnit;
	shift = 8 - shift;
	shift -= shiftUnit;
	x >>= shiftX;
	x += pos.y * stride;
	auto ptr = pPix + x;
	*ptr &= ~(maskVal << shift);
	*ptr |= colorIndex << shift;
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
