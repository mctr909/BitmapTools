#ifndef __OUTLINE_H__
#define __OUTLINE_H__

#include <vector>
#include "types.h"

class Bitmap;

class Outline {
private:
	enum struct E_DIRECTION : int32_t {
		BOTTOM_L = 0,
		BOTTOM,
		BOTTOM_R,
		LEFT,
		CENTER,
		RIGHT,
		TOP_L,
		TOP,
		TOP_R
	};
#pragma pack(push, 1)
	struct CELL {
		bool  filled;
		bool  traced;
		point pos;
		bool  *pFillAround[9];
	};
#pragma pack(pop)

private:
	uint8_t colorWhite;
	uint8_t colorBlack;
	int32_t width;
	int32_t height;
	int32_t pixelCount;
	CELL    *pCells = nullptr;
	Bitmap  *pBmp = nullptr;

public:
	/**
	 * アウトライン作業領域を確保します
	 * @param[in/out] pBmp ビットマップ
	 */
	Outline(Bitmap *pBmp);
	~Outline();

public:
	/*
	 * ビットマップを読込みます
	 * @params[in] lowerLum 下限輝度
	 * @returns 塗部の輝度
	 */
	double Read(const double lowerLum);
	/**
	 * アウトラインをビットマップに書出します
	 */
	void Write() const;
	/**
	 * アウトラインの頂点リストを作成します
	 * @returns 頂点リスト
	 */
	vector<vector<vec3>> CreatePolyline();

private:
	vector<vec3> EliminatePointsOnStraightLine(vector<point> polyline) const;

private:
	uint32_t GetIndex(point const &pos) const;
	uint32_t GetIndex(point const &pos, direction const &dir) const;

private:
	static bool HasFilledCell(bool **ppAround, E_DIRECTION const &dir);
};

inline uint32_t
Outline::GetIndex(point const &pos) const {
	int32_t x = pos.x;
	int32_t y = pos.y;
	int32_t isValid = x < width;
	isValid &= y < height;
	x += y * width;
	x *= isValid;
	x += isValid - 1;
	return x;
}

inline uint32_t
Outline::GetIndex(point const &pos, direction const &dPos) const {
	int32_t x = pos.x + dPos.x;
	int32_t y = pos.y + dPos.y;
	int32_t isValid = x >= 0;
	isValid &= y >= 0;
	isValid &= x < width;
	isValid &= y < height;
	x += y * width;
	x *= isValid;
	x += isValid - 1;
	return x;
}

inline bool
Outline::HasFilledCell(bool **ppAround, E_DIRECTION const &dir) {
	auto pFill = ppAround[static_cast<int32_t>(dir)];
	return nullptr == pFill ? false : *pFill;
}
#endif //__OUTLINE_H__
