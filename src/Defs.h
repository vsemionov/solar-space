#ifndef defs_h_incl
#define defs_h_incl


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

#ifdef _DEBUG
#define DEBUG TRUE
//#define USE_ZLOG
#else
#define DEBUG FALSE
#endif

#ifdef USE_ZLOG
#include <zlog.h>
#pragma comment(lib,"zlog.lib")
#endif


#endif
