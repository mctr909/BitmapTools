#ifndef __WORK_TABLE_H__
#define __WORK_TABLE_H__

#include "types.h"
#include "Bitmap.h"

#define DEFINE_COLOR_ON  (0x00)
#define DEFINE_COLOR_OFF (0xFF)

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
	struct type_workcell {
		byte data;
		Bitmap::position pos;
		uint32 index_bmp;
		uint32 index_dir[9];
	};
#pragma pack(pop)

struct type_worktable {
	type_workcell* pCells;
	byte color_on;
	byte color_off;
	int32 error;
};

void fn_worktable_create(type_worktable*, Bitmap& const);

extern inline type_workcell fn_worktable_get_data(uint32, E_DIRECTION, const type_worktable*, uint32);

#endif //__WORK_TABLE_H__
