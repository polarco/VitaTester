#include "runtime.h"
#include <psp2/kernel/threadmgr.h>
#include <stdatomic.h>
static atomic_bool enabled,finish,active[3];
static atomic_uint progress[3],checksum[3];
static SceUID threads[3]={-1,-1,-1};
static int worker(SceSize size,void *arg) {
    (void)size;int index=*(int *)arg;
    uint32_t x=0x12345678u+(unsigned)index;float f=1.001f;
    while(!atomic_load(&finish)) {
        if(!atomic_load(&enabled)) {sceKernelDelayThread(4000);continue;}
        atomic_store(&active[index],true);
        if(!atomic_load(&enabled)) {atomic_store(&active[index],false);continue;}
        for(unsigned j=0;j<4096;j++) {
            x^=x<<13; x^=x>>17; x^=x<<5;
            f=f*0.99991f+(float)(x&255)*0.00001f;
        }
        atomic_store(&checksum[index],x^(uint32_t)(f*100000));
        atomic_fetch_add(&progress[index],1);
        atomic_store(&active[index],false);
    }
    atomic_store(&active[index],false);
    return 0;
}
int vt_workers_start(void) {
    for(int i=0;i<3;i++) {
        threads[i]=sceKernelCreateThread("vt_stress",worker,VT_WORK_PRIORITY,16384,0,SCE_KERNEL_CPU_MASK_USER_0<<i,NULL);
        if(threads[i]<0 || sceKernelStartThread(threads[i],sizeof(i),&i)<0) return -1;
    }return 0;
}
SceUID vt_worker_id(int i) {return threads[i];}
unsigned vt_worker_progress(int i) {return atomic_load(&progress[i]);}
bool vt_workers_idle(void) {
    return !atomic_load(&enabled) && !atomic_load(&active[0]) && !atomic_load(&active[1]) && !atomic_load(&active[2]);
}
void vt_workers_enable(bool on) {atomic_store(&enabled,on);}
void vt_workers_finish(void) {
    atomic_store(&enabled,false);atomic_store(&finish,true);
    for(int i=0;i<3;i++) if(threads[i]>=0) {SceUInt timeout=500000;sceKernelWaitThreadEnd(threads[i],NULL,&timeout);}
}
