#ifndef __TYPES_H__
#define __TYPES_H__

typedef unsigned char  byte;
typedef unsigned short uint16;
typedef unsigned int   uint32;
typedef signed char    sbyte;
typedef short int16;
typedef int   int32;

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

#endif //__TYPES_H__
