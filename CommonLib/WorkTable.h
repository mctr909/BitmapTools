#ifndef __WORK_TABLE_H__
#define __WORK_TABLE_H__

#include <vector>
#include "Bitmap.h"

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
	bool traced;
	point pos;
	uint32 index_around[9];
};
#pragma pack(pop)

struct type_worktable {
	type_workcell* pCells;
	byte color_white;
	byte color_black;
	byte color_on;
	byte color_off;
	int32 error;
};

struct type_worktable_vert_info {
	double distance;
	bool deleted;
};

struct type_nest_info {
	uint32 parent;
	uint32 depth;
};

void
worktable_create(type_worktable*, Bitmap&);

void
worktable_write_outline(type_worktable&, Bitmap*);

vector<vector<point>>
worktable_create_polyline(type_worktable*, Bitmap&);

double
worktable_create_polygon(vector<point>&, vector<uint32>&, vector<surface>*, int32 order);

bool
worktable_inner_polygon(vector<surface>&, vector<surface>&, vector<point>&);

inline type_workcell
worktable_get_data(
    uint32      center_index,
    E_DIRECTION direction,
    type_worktable& table,
    uint32 size_max
) {
    type_workcell ret = {
        false,
		false,
        { INT32_MAX, INT32_MAX },
        {
            UINT32_MAX, UINT32_MAX, UINT32_MAX,
            UINT32_MAX, UINT32_MAX, UINT32_MAX,
            UINT32_MAX, UINT32_MAX, UINT32_MAX
        }
    };

    if (center_index >= size_max) {
        return (ret);
    }

    auto index = table.pCells[center_index].index_around[static_cast<uint32>(direction)];
    if (index < size_max) {
        ret = table.pCells[index];
    }

    return (ret);
}

inline bool
worktable_inner_triangle(point va, point vo, point vb, point p) {
	point oq, op;
	oq.x = va.x - vb.x;
	oq.y = va.y - vb.y;
	op.x = p.x - vb.x;
	op.y = p.y - vb.y;
	int32 normal_abp = oq.x * op.y - oq.y * op.x;
	oq.x = vo.x - va.x;
	oq.y = vo.y - va.y;
	op.x = p.x - va.x;
	op.y = p.y - va.y;
	int32 normal_oap = oq.x * op.y - oq.y * op.x;
	oq.x = vb.x - vo.x;
	oq.y = vb.y - vo.y;
	op.x = p.x - vo.x;
	op.y = p.y - vo.y;
	int32 normal_bop = oq.x * op.y - oq.y * op.x;
	if (normal_abp < 0 && normal_oap < 0 && normal_bop < 0) {
		return true;
	}
	if (normal_abp > 0 && normal_oap > 0 && normal_bop > 0) {
		return true;
	}
	return false;
}

inline bool
worktable_has_intersection(point a, point b, point c, point d) {
	auto ab_x = b.x - a.x;
	auto ab_y = b.y - a.y;
	auto cd_x = d.x - c.x;
	auto cd_y = d.y - c.y;
	auto denomi = ab_x * cd_y - ab_y * cd_x;
	if (0.0 == denomi) {
		return false;
	}
	auto ac_x = c.x - a.x;
	auto ac_y = c.y - a.y;
	auto s = (ac_x * cd_y - ac_y * cd_x) / denomi;
	if (s < 0.0 || 1.0 < s) {
		return false;
	}
	auto ca_x = a.x - c.x;
	auto ca_y = a.y - c.y;
	auto t = (ab_x * ca_y - ab_y * ca_x) / denomi;
	if (t < 0.0 || 1.0 < t) {
		return false;
	}
	return true;
}

#endif //__WORK_TABLE_H__
