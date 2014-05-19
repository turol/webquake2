#ifndef RW_LINUX_H
#define RW_LINUX_H


#include "../client/ref.h"

extern void (*KBD_Update_fp)(void);
extern void (*KBD_Init_fp)(Key_Event_fp_t fp);
extern void (*KBD_Close_fp)(void);


#endif  // RW_LINUX_H
