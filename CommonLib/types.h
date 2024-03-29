#ifndef __TYPES_H__
#define __TYPES_H__

#define INVALID_INDEX 0x7fffffffi32
#define INVALID_POS   0x7fffffffi32

typedef unsigned char  byte;
typedef unsigned short uint16;
typedef unsigned int   uint32;
typedef unsigned long  uint64;
typedef signed char    sbyte;
typedef short int16;
typedef int   int32;
typedef long  int64;

#pragma pack(push, 8)
struct point {
    int32 x;
    int32 y;
};
#pragma pack(pop)

#pragma pack(push, 8)
struct point_d {
    double x;
    double y;
};
#pragma pack(pop)

#pragma pack(push, 2)
struct point_b {
    sbyte x;
    sbyte y;
};
#pragma pack(pop)

#pragma pack(push, 4)
struct surface {
    uint32 a;
    uint32 o;
    uint32 b;
};
#pragma pack(pop)

#endif //__TYPES_H__
