#include "queue.h"
bool vt_queue_push(VtQueue *q,const VtRecord *r) {
    unsigned head=atomic_load_explicit(&q->head,memory_order_relaxed);
    if(head-atomic_load_explicit(&q->tail,memory_order_acquire)>=VT_QUEUE_CAP) return false;
    q->items[head%VT_QUEUE_CAP]=*r;
    atomic_store_explicit(&q->head,head+1,memory_order_release);return true;
}
bool vt_queue_pop(VtQueue *q,VtRecord *r) {
    unsigned tail=atomic_load_explicit(&q->tail,memory_order_relaxed);
    if(tail==atomic_load_explicit(&q->head,memory_order_acquire)) return false;
    *r=q->items[tail%VT_QUEUE_CAP];
    atomic_store_explicit(&q->tail,tail+1,memory_order_release);return true;
}
