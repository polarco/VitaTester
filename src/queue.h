#ifndef VT_QUEUE_H
#define VT_QUEUE_H
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#define VT_QUEUE_CAP 256
#define VT_RECORD_SIZE 2048
typedef struct { char text[VT_RECORD_SIZE]; unsigned len,seq; uint64_t queued; } VtRecord;
// Exactly one producer (capture), one consumer (logger). No disk lock in capture.
typedef struct { VtRecord items[VT_QUEUE_CAP]; atomic_uint head,tail; } VtQueue;
bool vt_queue_push(VtQueue *,const VtRecord *);
bool vt_queue_pop(VtQueue *,VtRecord *);
#endif
