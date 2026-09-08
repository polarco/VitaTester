// Compile the production capture/control module against fake VitaSDK entry points.
#include "../src/capture.c"
#include <assert.h>
#include "module.h"
extern const VtModule vt_module_stress;
void vt_original_draw(vita2d_font *f,const VtSnapshot *s,unsigned fps,bool stress){(void)f;(void)s;(void)fps;(void)stress;}
static int mhz[3]={333,111,166},logger_state=1,bad_priority,queue_full,waits;
static bool workers_on;
static bool fail_restore;
static unsigned worker_starts;
static uint64_t fake_now=1000000;
static FILE *records;
static int scenario; // 1 overlay, 2 logger failure, 3 suspend/resume callback, 4 stale API
int scePowerGetArmClockFrequency(void){return mhz[0];}
int scePowerGetGpuClockFrequency(void){return mhz[1];}
int scePowerGetBusClockFrequency(void){return mhz[2];}
int scePowerSetArmClockFrequency(int n){if(fail_restore && n==333)return -5;mhz[0]=n;return 0;}
int scePowerSetGpuClockFrequency(int n){mhz[1]=n;return 0;}
int scePowerSetBusClockFrequency(int n){mhz[2]=n;return 0;}
int scePowerGetBatteryTemp(void){return 3250;}
int scePowerGetBatteryLifePercent(void){return 80;}
int scePowerIsPowerOnline(void){return 1;}
int scePowerIsSuspendRequired(void){return 0;}
int scePowerRegisterCallback(int id){(void)id;return 0;}
int scePowerUnregisterCallback(int id){(void)id;return 0;}
int sceKernelCreateCallback(const char *n,int x,int (*f)(int,int,int,void *),void *p){(void)n;(void)x;(void)f;(void)p;return 9;}
int sceKernelDeleteCallback(int n){(void)n;return 0;}
int sceKernelCheckCallback(void){
    if(scenario==5 && fake_now>=1400000) {
        fail_restore=fake_now<1600000;
        vt_runtime_stress(false);
        if(fake_now<1700000)assert(!vt_module_stress.leave(&live));
    }
    if(scenario==3 && fake_now>=1400000 && fake_now<1408000)power_callback(0,0,SCE_POWER_CB_SYSTEM_SUSPEND,NULL);
    if(scenario==3 && fake_now>=1600000 && fake_now<1608000)power_callback(0,0,SCE_POWER_CB_SYSTEM_RESUME,NULL);
    return 0;
}
int sceKernelGetThreadInfo(int id,SceKernelThreadInfo *i){
    int p[]={64,80,96,160,160,160};if(id<0||id>5)return -1;
    i->currentPriority=bad_priority?160:p[id];i->currentCpuAffinityMask=id>=3?1<<(id-3):7;return 0;
}
int sceKernelGetThreadId(void){return 1;}
int sceKernelChangeThreadPriority(int id,int p){(void)id;(void)p;return 0;}
int sceKernelCreateThread(const char *n,int (*f)(SceSize,void *),int p,unsigned sz,unsigned at,int mask,void *opt){(void)n;(void)f;(void)p;(void)sz;(void)at;(void)mask;(void)opt;return 0;}
int sceKernelStartThread(int id,unsigned n,void *p){(void)id;(void)n;(void)p;return 0;}
int sceKernelWaitThreadEnd(int id,void *p,unsigned *t){(void)id;(void)p;(void)t;assert(!workers_on && mhz[0]==333);waits++;return 0;}
int sceKernelCreateMutex(const char *n,unsigned a,int c,void *o){(void)n;(void)a;(void)c;(void)o;return 8;}
int sceKernelTryLockMutex(int id,int n){(void)id;(void)n;return 0;}
int sceKernelUnlockMutex(int id,int n){(void)id;(void)n;return 0;}
int sceKernelDelayThreadCB(unsigned us){if(scenario>=1 && scenario<=3 && fake_now>=1650000)assert(!live.running && !workers_on && mhz[0]==333);fake_now+=us;if(fake_now>=2200000)atomic_store(&finish,true);return 0;}
uint64_t sceKernelGetProcessTimeWide(void){return fake_now;}
int _sceAppMgrGetAppState(SceAppMgrAppState *a,size_t s,unsigned v){(void)s;(void)v;a->isSystemUiOverlaid=scenario==1 && fake_now>=1400000 && fake_now<1600000;return 0;}
int sceAppMgrReceiveSystemEvent(SceAppMgrSystemEvent *e){e->systemEvent=SCE_APPMGR_SYSTEMEVENT_ON_RESUME;return 0;}
int sceCtrlGetButtonIntercept(int *p){*p=0;return 0;}
int sceCtrlSetSamplingMode(int m){(void)m;return 0;}
int sceTouchSetSamplingState(int p,int m){(void)p;(void)m;return 0;}
int sceCtrlPeekBufferPositive(int p,SceCtrlData *c,int n){(void)p;(void)n;*c=(SceCtrlData){.timeStamp=(scenario==4 && fake_now>=1400000)?1400000:fake_now,.lx=128,.ly=128,.rx=128,.ry=128};return 1;}
int sceTouchPeek(int p,SceTouchData *t,unsigned n){
    (void)n;memset(t,0,sizeof(*t));t->timeStamp=fake_now;
    if(p==0 && fake_now>=1200000 && fake_now<1216000){t->reportNum=1;t->report[0].x=100;t->report[0].y=950;}
    return 1;
}
int vt_logger_start(void){return 0;}
SceUID vt_logger_id(void){return 2;}
int vt_logger_state(void){if(scenario==2 && fake_now>=1400000)logger_state=-1;return logger_state;}
void vt_logger_finish(void){assert(!workers_on && mhz[0]==333);}
void vt_log_status(unsigned *a,unsigned *b,unsigned *c){*a=sequence?sequence-1:0;*b=2;*c=0;}
bool vt_log_enqueue(const char *s,unsigned n,unsigned seq,uint64_t now){(void)seq;(void)now;if(queue_full)return false;if(records)assert(fwrite(s,1,n,records)==n);return true;}
int vt_workers_start(void){return 0;}
SceUID vt_worker_id(int n){return 3+n;}
unsigned vt_worker_progress(int n){(void)n;return workers_on?100:0;}
bool vt_workers_idle(void){return !workers_on && !(scenario==5 && fake_now<1700000);}
void vt_workers_enable(bool on){if(on && !workers_on)worker_starts++;workers_on=on;}
void vt_workers_finish(void){assert(!workers_on && mhz[0]==333);}
static void prepare(void){
    memset(&live,0,sizeof(live));memset(&clocks,0,sizeof(clocks));memset(stamp,0,sizeof(stamp));memset(seen,0,sizeof(seen));
    memset(histogram,0,sizeof(histogram));polls=0;last_poll=last_sample=0;
    worker_starts=0;session_logged=touch_down=false;sequence=0;logger_state=1;fake_now=1000000;atomic_store(&finish,false);
    assert(vt_runtime_init()==0);vt_runtime_stress(true);
}
int main(void){
    records=fopen("build-host/runtime.jsonl","w");assert(records);
    for(scenario=1;scenario<=6;scenario++){
        prepare();if(scenario==6)vt_runtime_stress(false);capture(0,NULL);assert(worker_starts==(scenario==6?0u:1u));
        if(scenario>=5)assert(vt_module_stress.leave(&live));
        assert(!workers_on && !live.running && mhz[0]==333 && mhz[1]==111 && mhz[2]==166);
        if(scenario==2)assert(live.log_failed);
        vt_runtime_shutdown();
    }
    scenario=0;prepare();live.now=fake_now;live.valid=true;session_logged=true;lifecycle_ok=true;
    live.front.reportNum=1;live.front.report[0].x=100;live.front.report[0].y=950;
    bad_priority=1;command();assert(!live.running && mhz[0]==333);bad_priority=0;
    touch_down=false;command();assert(live.running && workers_on && mhz[0]==444);
    // A valid front command still stops while another input API is failing.
    live.valid=false;touch_down=false;command();assert(!live.running && mhz[0]==333);
    live.valid=true;touch_down=false;queue_full=1;command();assert(live.log_failed && !live.running && !workers_on && mhz[0]==333);
    queue_full=0;
    // Commands belonging to stress cannot start workers from menu/Input/Scanner.
    live.log_failed=false;live.valid=true;session_logged=true;touch_down=false;
    vt_runtime_stress(false);unsigned before=worker_starts;command();
    assert(!live.running && worker_starts==before);
    vt_runtime_stress(true);touch_down=false;command();assert(live.running);
    fail_restore=true;vt_runtime_stress(false);stop_now();
    assert(!workers_on && live.restore_failed && clocks.saved);
    touch_down=false;command();assert(!live.running);
    fail_restore=false;stop_now();assert(!live.restore_failed && !clocks.saved && mhz[0]==333);
    fclose(records);
    assert(waits==6);puts("runtime: actual control loop, overlay, callbacks, logger failure, priorities, saturation, stop with API error OK");
    return 0;
}
