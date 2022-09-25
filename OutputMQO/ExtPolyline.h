#ifndef __EXT_POLYLINE_H__
#define __EXT_POLYLINE_H__

#include "../Outline/Bitmap.h"

#include "mqo.h"

struct type_mngmqo {
	unsigned long index_vertex;
	unsigned long index_face[9];
};

type_mqo_object fn_convert_table_to_mqo(const Bitmap& const);
uint32 fn_support_mqo_create_vertex(const Bitmap::position, type_mqo_object*);
uint32 fn_support_mqo_create_face(const uint32, const uint32, type_mqo_object*);

#endif //__EXT_POLYLINE_H__
