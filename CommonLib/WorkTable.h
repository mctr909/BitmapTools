#ifndef __WORK_TABLE_H__
#define __WORK_TABLE_H__

#include <vector>
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
		bool enable;
		point pos;
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

vector<vector<point>>
fn_worktable_outline(Bitmap*, type_worktable*);

void
fn_worktable_create_polygon(vector<point> vert, vector<uint32>* pIndex, vector<vector<uint32>>* pSurf);

inline type_workcell
fn_worktable_get_data(
    uint32      center_index,
    E_DIRECTION direction,
    const type_worktable* ptable,
    uint32 size_max
) {
    type_workcell ret = {
        false,
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

inline bool
fn_worktable_inner_triangle(point va, point vo, point vb, point p) {
	point oa, op;
	oa.x = va.x - vb.x;
	oa.y = va.y - vb.y;
	op.x = p.x - vb.x;
	op.y = p.y - vb.y;
	double na = oa.x * op.y - oa.y * op.x;
	oa.x = vo.x - va.x;
	oa.y = vo.y - va.y;
	op.x = p.x - va.x;
	op.y = p.y - va.y;
	double nb = oa.x * op.y - oa.y * op.x;
	oa.x = vb.x - vo.x;
	oa.y = vb.y - vo.y;
	op.x = p.x - vo.x;
	op.y = p.y - vo.y;
	double nc = oa.x * op.y - oa.y * op.x;
	if (na < 0 && nb < 0 && nc < 0) {
		return true;
	}
	if (na > 0 && nb > 0 && nc > 0) {
		return true;
	}
	return false;
}

#endif //__WORK_TABLE_H__
