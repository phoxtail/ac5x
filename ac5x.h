#ifndef AC5X_H
#define AC5X_H

#include <stdbool.h>

typedef unsigned char U8;
typedef char S8;
typedef unsigned short U16;
typedef unsigned long U32;
typedef unsigned long BOOL;

#define AC5X_IGNORE(toIgnore)

#ifndef NULL
#define NULL 0
#endif

#ifndef MAX_UX
#define MAX_UX

#define MAX_U8  0xFF
#define MAX_U16 0xFFFF
#define MAX_U24 0xFFFFFF
#define MAX_U32 0xFFFFFFFF

#define FALSE false
#define TRUE true

#define AC5X_OK 0
#define AC5X_FAIL -1

#endif

#define AC5X_DEVICE_MAX 1
#define MAX_AC5X_CHANNEL 1

#endif

