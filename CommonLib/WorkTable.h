#ifndef __WORK_TABLE_H__
#define __WORK_TABLE_H__

#include <vector>
#include "types.h"

class Bitmap;

class WorkTable {
private:
#pragma pack(push, 1)
	struct Cell {
		bool filled;
		bool traced;
		point pos;
		uint32 index_around[9];
	};
#pragma pack(pop)

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
	static const int32 TRACE_RADIUS = 3;
	static const int32 TRACE_DIRS = 8;
	static const int32 PREFER_DIRS = 8;
	static const sbyte PREFER_DIR[PREFER_DIRS];
	static const point_b TRACE_DIR[TRACE_RADIUS][TRACE_DIRS];
	static const Cell DEFAULT_CELL;

private:
	byte m_color_white;
	byte m_color_black;
	int32 m_width;
	int32 m_height;
	uint32 m_pixel_count;
	Cell* mp_cells;

public:
	WorkTable(int32 width, int32 height);
	~WorkTable();

public:
	double Setup(Bitmap& bmp, double lum_limit);
	void WriteOutline(Bitmap* pBmp, int32 line_weight);
	vector<vector<point>> CreatePolyline();

private:
	void eliminatePointsOnStraightLine(vector<point>* pPolyline);

private:
	inline Cell get_cell(uint32 center_index, E_DIRECTION direction) {
		if (center_index >= m_pixel_count) {
			return DEFAULT_CELL;
		}
		auto index = mp_cells[center_index].index_around[static_cast<uint32>(direction)];
		if (INVALID_INDEX == index) {
			return DEFAULT_CELL;
		}
		return mp_cells[index];
	}
	inline uint32 get_index(point pos) {
		if ((pos.x >= m_width) || (pos.y >= m_height)) {
			return INVALID_INDEX;
		}
		return ((pos.x + (m_width * pos.y)));
	}
	inline uint32 get_index_ofs(point pos, int32 dx, int32 dy) {
		auto x = pos.x + dx;
		auto y = pos.y + dy;
		if ((x < 0) || (x >= m_width) || (y < 0) || (y >= m_height)) {
			return INVALID_INDEX;
		}
		return (x + (m_width * y));
	}
};

#endif //__WORK_TABLE_H__
