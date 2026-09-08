#ifndef VT_NAVIGATION_H
#define VT_NAVIGATION_H
#include <stdint.h>
#include <stdbool.h>
typedef struct {uint64_t since,last;bool latched;} VtGesture;
bool vt_gesture(VtGesture *,uint64_t now,bool valid,unsigned total,unsigned central);
#endif
