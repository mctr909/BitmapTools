#ifndef __DECLEASE_H__
#define __DECLEASE_H__

#include "types.h"
#include "Bitmap.h"

#define DEFINE_HUE_RANGE        (16)
#define DEFINE_LIGHTNESS_RANGE  (32)
#define DEFINE_SATURATION_RANGE (7)

#pragma pack(push, 4)
struct type_histogram {
	byte src_h;
	byte src_s;
	byte src_l;
	byte dst_h;
	byte dst_s;
	byte dst_l;
	uint16 reserved;
	uint32 count;
};
#pragma pack(pop)

#pragma pack(push, 4)
struct type_hsl_tops {
	byte h;
	byte s;
	byte l;
	byte reserved;
	uint32 count;
	uint32 index;
};
#pragma pack(pop)

Bitmap* fn_exec_declease(Bitmap& const);
void fn_calc_histogram(type_histogram*);
inline void fn_rgb2hsl(type_histogram*, Bitmap::pix24*);
inline void fn_hsl2rgb(Bitmap::pix24*, byte, byte, byte);

#endif //__DECLEASE_H__
