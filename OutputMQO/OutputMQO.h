#ifndef __EXT_POLYLINE_H__
#define __EXT_POLYLINE_H__

#include "../CommonLib/Bitmap.h"
#include "mqo.h"

//#define DEBUG_OUTPUT_MQO ()

struct type_mngmqo {
	unsigned long index_vertex;
	unsigned long index_face[9];
};

type_mqo_object fn_output_mqo_exec(Bitmap* pBmp);

#endif //__EXT_POLYLINE_H__