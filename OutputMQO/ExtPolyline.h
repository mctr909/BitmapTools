#ifndef __EXT_POLYLINE_H__
#define __EXT_POLYLINE_H__

#include "../CommonLib/Bitmap.h"
#include "mqo.h"

struct type_mngmqo {
	unsigned long index_vertex;
	unsigned long index_face[9];
};

type_mqo_object fn_convert_table_to_mqo(Bitmap*);

#endif //__EXT_POLYLINE_H__
