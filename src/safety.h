#ifndef VT_SAFETY_H
#define VT_SAFETY_H
#include <stdbool.h>
#include <stddef.h>
typedef struct {
    int (*get)(void *, int);
    int (*set)(void *, int, int);
    void *opaque;
    int baseline[3], setter_rc[3], restore_rc[3], effective[3];
    bool saved, running;
} VtClocks;
bool vt_clocks_start(VtClocks *);
bool vt_clocks_stop(VtClocks *);
typedef int (*VtWrite)(void *,const void *,unsigned);
typedef int (*VtSync)(void *);
bool vt_write_record(VtWrite,VtSync,void *,const char *,size_t);
#endif
