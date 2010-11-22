#ifndef _defs_h_incl_
#define _defs_h_incl_


#ifndef __cplusplus
typedef enum{false,true} bool;
#endif

#define byte unsigned char
#define sbyte signed char
#define word unsigned short
#define sword signed short
#define dword unsigned long
#define sdword signed long
#define qword unsigned _int64
#define sqword signed _int64
#define bbool unsigned char
#define dwbool unsigned long


#define CLAMP(val,valmin,valmax) { val=max(min(val,valmax),valmin); }

#define TEAMNAME "vs"
#define APPNAME  "Solar Space"
#define APPNAMEZ "Solar Space\0"

#define M_E         2.71828182845904523536
#define M_LOG2E     1.44269504088896340736
#define M_LOG10E    0.434294481903251827651
#define M_LN2       0.693147180559945309417
#define M_LN10      2.30258509299404568402
#define M_PI        3.14159265358979323846
#define M_PI_2      1.57079632679489661923
#define M_PI_4      0.785398163397448309616
#define M_1_PI      0.318309886183790671538
#define M_2_PI      0.636619772367581343076
#define M_1_SQRTPI  0.564189583547756286948
#define M_2_SQRTPI  1.12837916709551257390
#define M_SQRT2     1.41421356237309504880
#define M_SQRT_2    0.707106781186547524401

#ifdef _DEBUG
#define DEBUG TRUE
#define USE_ZLOG
#else
#define DEBUG FALSE
#endif

#ifdef USE_ZLOG
#include <zlog.h>
#pragma comment(lib,"zlog.lib")
#endif


#endif
