#ifndef __OUTLINE_H__
#define __OUTLINE_H__

#include <vector>
#include "types.h"

class Bitmap;

class Outline {
private:
	enum struct E_DIRECTION {
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
		CELL  *pAround[9];
	};
#pragma pack(pop)

private:
	uint8_t colorWhite;
	uint8_t colorBlack;
	int32_t width;
	int32_t height;
	int32_t stride;
	int32_t pixelCount;
	CELL    *pCells = nullptr;

public:
	/* 指定されたサイズのアウトラインを作成します */
	Outline(const int32_t width, const int32_t height);
	~Outline();

public:
	/* ビットマップを読込みます */
	double Read(Bitmap const &bmp, const double lumLimit);
	/* アウトラインをビットマップに書出します */
	void Write(Bitmap *pBmp);
	/* アウトラインの頂点リストを作成します */
	vector<vector<vec3>> CreatePolyline();

private:
	vector<vec3> EliminatePointsOnStraightLine(vector<point> polyline);

private:
	bool GetCell(const int32_t index, const E_DIRECTION dir);
	uint32_t GetIndex(point const &pos);
	uint32_t GetIndex(point const &pos, direction const &dir);
};

inline bool
Outline::GetCell(const int32_t index, const E_DIRECTION dir) {
	int32_t x = index;
	int32_t isValid = x >= 0;
	isValid &= x < pixelCount;
	x *= isValid;
	auto pCell = pCells[x].pAround[static_cast<int32_t>(dir)];
	isValid = nullptr != pCell;
	return *reinterpret_cast<uint8_t*>(pCell) * isValid;
}

inline uint32_t
Outline::GetIndex(point const &pos) {
	int32_t x = pos.x;
	int32_t y = pos.y;
	int32_t isValid = x < width;
	isValid &= y < height;
	x += y * stride;
	x *= isValid;
	x += isValid - 1;
	return x;
}

inline uint32_t
Outline::GetIndex(point const &pos, direction const &dir) {
	int32_t x = pos.x + dir.x;
	int32_t y = pos.y + dir.y;
	int32_t isValid = x >= 0;
	isValid &= y >= 0;
	isValid &= x < width;
	isValid &= y < height;
	x += y * stride;
	x *= isValid;
	x += isValid - 1;
	return x;
}
#endif //__OUTLINE_H__
