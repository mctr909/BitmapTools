#ifndef __EXT_OUTLINE_H__
#define __EXT_OUTLINE_H__

#include "types.h"
#include "Bitmap.h"

#define DEFINE_COLOR_ON  (0x00)
#define DEFINE_COLOR_OFF (0xFF)

enum struct E_DIRECTION {
	BOTTOM_L = 0, // ����
	BOTTOM,       // ��
	BOTTOM_R,     // �E��
	LEFT,         // ��
	CENTER,       // ����
	RIGHT,        // �E
	TOP_L,        // ����
	TOP,          // ��
	TOP_R,        // �E��
};

#pragma pack(push, 1)
struct type_workcell {
	byte data;
	uint32 x;
	uint32 y;
	uint32 index_bmp;
	uint32 index_dir[9];
};
#pragma pack(pop)

struct type_worktable {
	type_workcell *pCells;
	byte color_on;
	byte color_off;
	int32 error;
};

Bitmap* fn_ext_outline(Bitmap& const);
void fn_thickness(Bitmap*, int32);
void fn_worktable_create(type_worktable*, Bitmap& const);
inline type_workcell fn_worktable_get_data(uint32, E_DIRECTION, const type_worktable*, uint32);

#endif //__EXT_OUTLINE_H__
