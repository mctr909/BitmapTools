#ifndef __WORK_TABLE_H__
#define __WORK_TABLE_H__

#include <vector>
#include "types.h"

class Bitmap;

enum struct E_DIRECTION {
	BOTTOM_L = 0, // ç∂â∫
	BOTTOM,       // â∫
	BOTTOM_R,     // âEâ∫
	LEFT,         // ç∂
	CENTER,       // íÜâõ
	RIGHT,        // âE
	TOP_L,        // ç∂è„
	TOP,          // è„
	TOP_R,        // âEè„
};

#pragma pack(push, 1)
struct TYPE_WORKCELL {
	bool filled;
	bool traced;
	point pos;
	uint32 index_around[9];
};
#pragma pack(pop)

struct TYPE_WORKTABLE {
	TYPE_WORKCELL* pCells;
	byte color_white;
	byte color_black;
	int32 width;
	int32 height;
	uint32 pixel_count;
};

void
worktable_free(TYPE_WORKTABLE* pTable);

bool
worktable_alloc(TYPE_WORKTABLE* pTable, int32 width, int32 height);

double
worktable_setup(TYPE_WORKTABLE* pTable, Bitmap& bmp, double lum_limit);

void
worktable_write_outline(TYPE_WORKTABLE& table, Bitmap* pBmp, int32 line_weight);

vector<vector<point>>
worktable_create_polyline(TYPE_WORKTABLE* pTable);

inline TYPE_WORKCELL
worktable_get_cell(
	TYPE_WORKTABLE& table,
	uint32          center_index,
	E_DIRECTION     direction
) {
	static const TYPE_WORKCELL DEFAULT_CELL = {
		false,
		false,
		{ INVALID_POS, INVALID_POS },
		{
			INVALID_INDEX, INVALID_INDEX, INVALID_INDEX,
			INVALID_INDEX, INVALID_INDEX, INVALID_INDEX,
			INVALID_INDEX, INVALID_INDEX, INVALID_INDEX
		}
	};
	if (center_index >= table.pixel_count) {
		return DEFAULT_CELL;
	}
	auto index = table.pCells[center_index].index_around[static_cast<uint32>(direction)];
	if (INVALID_INDEX == index) {
		return DEFAULT_CELL;
	}
	return table.pCells[index];
}

inline uint32
worktable_get_index(TYPE_WORKTABLE& table, point pos) {
	if ((pos.x >= table.width) || (pos.y >= table.height)) {
		return INVALID_INDEX;
	}
	return ((pos.x + (table.width * pos.y)));
}

inline uint32
worktable_get_index_ofs(TYPE_WORKTABLE& table, point pos, int32 dx, int32 dy) {
	auto x = pos.x + dx;
	auto y = pos.y + dy;
	if ((x < 0) || (x >= table.width) || (y < 0) || (y >= table.height)) {
		return INVALID_INDEX;
	}
	return (x + (table.width * y));
}

#endif //__WORK_TABLE_H__
