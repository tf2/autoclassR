#ifdef _WIN32
#include <winsock.h> /* has select decl in Gcc-win32 b18*/
#endif

/* #define TRUE    1 */
/* #define FALSE   0 */
/* allow for \n & \0 in addition to 100 characters */
#define LINLIM 102

#ifndef _MSC_VER
typedef unsigned int BOOLEAN;
#endif

#define MAXPARAMS 40
#define PARAMNAMLEN 35

/* aju 980612: Prefixed enum members with T so they would not clash with Win32 
    predefined types. This affects getparams.c, intf-reports.c, and
    search-control.c*/
typedef enum {TSTRING, TBOOL, TINT, TFLOAT, TDOUBLE, TINT_LIST} PARAMTYPE;

/* PARAMTYPE */
/*   11jan 95: add paramptr_overridden & overridden_p */
 
typedef struct /*parameters*/
{
  PARAMTYPE paramtype;
  char paramname[PARAMNAMLEN];
  void *paramptr;
  void *paramptr_overridden;
  int overridden_p;
  int max_length;          /* for paramtypes INT_LIST & STRING */
} PARAM, *PARAMP;
