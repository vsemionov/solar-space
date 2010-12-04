#ifndef defs_h_incl
#define defs_h_incl


#ifndef __cplusplus
typedef enum{false,true} bool;
#endif


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

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif


#endif
