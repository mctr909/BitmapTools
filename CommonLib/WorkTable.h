#ifndef __WORK_TABLE_H__
#define __WORK_TABLE_H__

#include "Bitmap.h"

#define DEFINE_COLOR_ON  (0x00)
#define DEFINE_COLOR_OFF (0xFF)

enum struct E_DIRECTION {
	BOTTOM_L = 0, // ¶‰º
	BOTTOM,       // ‰º
	BOTTOM_R,     // ‰E‰º
	LEFT,         // ¶
	CENTER,       // ’†‰›
	RIGHT,        // ‰E
	TOP_L,        // ¶ã
	TOP,          // ã
	TOP_R,        // ‰Eã
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

inline type_workcell
fn_worktable_get_data(
    uint32      center_index,
    E_DIRECTION direction,
    const type_worktable* ptable,
    uint32 size_max
) {
    type_workcell ret = {
        (*ptable).color_off,
        { INT32_MAX, INT32_MAX },
        UINT32_MAX,
        {
            UINT32_MAX, UINT32_MAX, UINT32_MAX,
            UINT32_MAX, UINT32_MAX, UINT32_MAX,
            UINT32_MAX, UINT32_MAX, UINT32_MAX
        }
    };

    if (center_index >= size_max) {
        return (ret);
    }

    auto index = (*ptable).pCells[center_index].index_dir[static_cast<uint32>(direction)];
    if (index < size_max) {
        ret = (*ptable).pCells[index];
    }

    return (ret);
}

#endif //__WORK_TABLE_H__
