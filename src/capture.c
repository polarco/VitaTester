#include "runtime.h"
#include "safety.h"
#include "queue.h"
#include <psp2/appmgr.h>
#include <psp2/power.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
static VtSnapshot live,published;
static VtClocks clocks;
static SceUID thread=-1,mutex=-1,ui_thread=-1;
static atomic_bool finish, stress_enabled, capture_focused;
static atomic_uint system_epoch;
static bool stress_previous;
void vt_runtime_stress(bool enabled) {atomic_store(&stress_enabled,enabled);}
unsigned vt_runtime_epoch(void) {return atomic_load(&system_epoch);}
bool vt_runtime_focused(void) {return atomic_load(&capture_focused);}
static atomic_uint fps;
static uint64_t started,stamp[3],seen[3],last_poll,last_sample;
static unsigned sequence,histogram[102],polls;
static char session[64];
static bool touch_down,session_logged,lifecycle_ok;
static int focus_rc,intercept_rc,intercept,overlay,callback_rc;
static int get_clock(void *ctx,int i) {
    (void)ctx;return i==0?scePowerGetArmClockFrequency():i==1?scePowerGetGpuClockFrequency():scePowerGetBusClockFrequency();
}
static int set_clock(void *ctx,int i,int v) {
    (void)ctx;return i==0?scePowerSetArmClockFrequency(v):i==1?scePowerSetGpuClockFrequency(v):scePowerSetBusClockFrequency(v);
}
static void stop_now(void) {
    vt_workers_enable(false);
    if(live.running) live.elapsed=live.now-started;
    live.running=false;
    live.restore_failed=!vt_clocks_stop(&clocks);
    for(int i=0;i<3;i++) live.clocks[i]=get_clock(NULL,i);
}
static void emit(void *ctx,const char *event,const char *input,const char *state,uint64_t now,uint64_t good) {
    (void)ctx;
    if(vt_logger_state()!=1 || (!session_logged && strcmp(event,"session")!=0)) return;
    char utc[32],line[VT_RECORD_SIZE],temperature[32];
    if(live.thermal_at) snprintf(temperature,sizeof(temperature),"%.2f",live.temp/100.0);
    else snprintf(temperature,sizeof(temperature),"null");
    time_t wall=time(NULL);struct tm tm;
    gmtime_r(&wall,&tm);strftime(utc,sizeof(utc),"%Y-%m-%dT%H:%M:%SZ",&tm);
    vt_log_status(&live.confirmed,&live.write_ms,&live.queue_delay_ms);
    int n=snprintf(line,sizeof(line),
      "{\"schema\":1,\"app\":\"1.5.1\",\"session\":\"%s\",\"seq\":%u,\"utc\":\"%s\",\"mono_us\":%llu,"
      "\"event\":\"%s\",\"input\":\"%s\",\"state\":\"%s\",\"last_good_us\":%llu,"
      "\"battery_temp_c\":%s,\"thermal_rc\":%d,\"thermal_age_us\":%llu,\"battery_pct\":%d,\"external_power\":%d,"
      "\"clocks_mhz\":[%d,%d,%d],\"clock_set_rc\":[%d,%d,%d],\"clock_restore_rc\":[%d,%d,%d],"
      "\"clock_baseline_mhz\":[%d,%d,%d],\"elapsed_us\":%llu,\"stress\":%s,\"capture_rc\":[%d,%d,%d],\"new_samples\":[%d,%d,%d],\"device_us\":[%llu,%llu,%llu],"
      "\"valid\":%s,\"focus_rc\":%d,\"intercept_rc\":%d,\"intercept\":%d,\"overlay\":%d,\"callback_rc\":%d,"
      "\"max_poll_us\":%llu,\"p99_poll_us\":%llu,\"fps\":%u,\"workers\":[%u,%u,%u],"
      "\"priorities\":[%d,%d,%d,%d,%d,%d],\"affinity\":[%d,%d,%d],"
      "\"last_confirmed_seq\":%u,\"max_write_ms\":%u,\"max_queue_delay_ms\":%u,\"buttons\":%u,\"axes\":[%u,%u,%u,%u],"
      "\"contacts\":[%u,%u],\"limits_s\":[%u,%u,%u,%u,%u]}\n",
      session,++sequence,utc,(unsigned long long)now,event,input,state,(unsigned long long)good,
      temperature,live.thermal_rc,(unsigned long long)(live.now-live.thermal_at),live.battery,live.online,
      live.clocks[0],live.clocks[1],live.clocks[2],clocks.setter_rc[0],clocks.setter_rc[1],clocks.setter_rc[2],
      clocks.restore_rc[0],clocks.restore_rc[1],clocks.restore_rc[2],
      clocks.baseline[0],clocks.baseline[1],clocks.baseline[2],(unsigned long long)live.elapsed,live.running?"true":"false",
      live.rc[0],live.rc[1],live.rc[2],live.fresh[0],live.fresh[1],live.fresh[2],
      (unsigned long long)stamp[0],(unsigned long long)stamp[1],(unsigned long long)stamp[2],live.valid?"true":"false",
      focus_rc,intercept_rc,intercept,overlay,callback_rc,(unsigned long long)live.max_poll,(unsigned long long)live.p99_poll,
      atomic_load(&fps),vt_worker_progress(0),vt_worker_progress(1),vt_worker_progress(2),
      live.priorities[0],live.priorities[1],live.priorities[2],live.priorities[3],live.priorities[4],live.priorities[5],
      live.affinities[0],live.affinities[1],live.affinities[2],live.confirmed,live.write_ms,live.queue_delay_ms,
      live.pad.buttons,live.pad.lx,live.pad.ly,live.pad.rx,live.pad.ry,live.front.reportNum,live.back.reportNum,
      live.diagnostic.limits.held_s,live.diagnostic.limits.idle_s,live.diagnostic.limits.step_s,
      live.diagnostic.limits.ghost_s,live.diagnostic.limits.rear_s);
    if(n<0 || n>=(int)sizeof(line) || !vt_log_enqueue(line,(unsigned)n,sequence,live.now)) {
        live.log_failed=true;stop_now();snprintf(live.status,sizeof(live.status),"Falha de registro: carga parada");
    }
}
static bool priorities(void) {
    SceUID ids[]={thread,ui_thread,vt_logger_id(),vt_worker_id(0),vt_worker_id(1),vt_worker_id(2)};
    for(int i=0;i<6;i++) {
        SceKernelThreadInfo info;memset(&info,0,sizeof(info));info.size=sizeof(info);
        if(ids[i]<0 || sceKernelGetThreadInfo(ids[i],&info)<0) return false;
        live.priorities[i]=info.currentPriority;
        if(i>=3) { live.affinities[i-3]=info.currentCpuAffinityMask;if(info.currentCpuAffinityMask!=(SCE_KERNEL_CPU_MASK_USER_0<<(i-3))) return false; }
    }
    return live.priorities[0]<live.priorities[1] && live.priorities[1]<live.priorities[2] &&
        live.priorities[2]<live.priorities[3] && live.priorities[2]<live.priorities[4] && live.priorities[2]<live.priorities[5];
}
static void lifecycle(const char *reason) {
    atomic_fetch_add(&system_epoch,1);
    stop_now(); live.valid=false;touch_down=true;
    snprintf(live.status,sizeof(live.status),"Evento do sistema: stress permanece parado");
    VtInput invalid={.now=live.now,.valid=false};vt_diag_feed(&live.diagnostic,&invalid);
    emit(NULL,"lifecycle","system",reason,live.now,0);
}
static int power_callback(int notify,int count,int flags,void *ctx) {
    (void)notify;(void)count;(void)ctx;
    unsigned mask=SCE_POWER_CB_SYSTEM_SUSPEND|SCE_POWER_CB_SYSTEM_RESUMING|SCE_POWER_CB_SYSTEM_RESUME|
        SCE_POWER_CB_APP_RESUME|SCE_POWER_CB_APP_SUSPEND|SCE_POWER_CB_APP_RESUMING;
    if((unsigned)flags & mask) lifecycle("power_suspend_or_resume");
    return 0;
}
static void command(void) {
    if(!atomic_load(&stress_enabled)) {touch_down=true;return;}
    if(live.front.reportNum==0) {touch_down=false;return;}
    if(touch_down) return;
    touch_down=true;
    int x=live.front.report[0].x*960/1920,y=live.front.report[0].y*544/1088;
    if(y<456 || y>=501) return;
    if(x<240) {
        if(live.running) {stop_now();snprintf(live.status,sizeof(live.status),"Stress parado pela tela");emit(NULL,"stress","all","stopped_touch",live.now,0);}
        else {
            live.priority_ok=priorities();
            if(!live.priority_ok || !lifecycle_ok || !live.valid || vt_logger_state()!=1 || !session_logged || live.log_failed) {
                snprintf(live.status,sizeof(live.status),"Inicio bloqueado: captura/prioridade/log/sistema");
                emit(NULL,"stress","all","start_blocked",live.now,0);return;
            }
            if(!vt_clocks_start(&clocks)) {
                live.restore_failed=clocks.saved;
                for(int i=0;i<3;i++) live.clocks[i]=get_clock(NULL,i);
                snprintf(live.status,sizeof(live.status),"Falha ao configurar clocks");
                emit(NULL,"clocks","all","start_failed",live.now,0);return;
            }
            for(int i=0;i<3;i++) live.clocks[i]=clocks.effective[i];
            live.running=true;started=live.now;live.elapsed=0;
            emit(NULL,"stress","all","started",live.now,0);
            if(!live.log_failed) {vt_workers_enable(true);snprintf(live.status,sizeof(live.status),"Stress ativo");}
        }
    } else if(x<480) vt_diag_mode(&live.diagnostic,!live.diagnostic.guided,live.now);
    else if(x<720) live.setting=(live.setting+1)%5;
    else {
        unsigned *limits[]={&live.diagnostic.limits.held_s,&live.diagnostic.limits.idle_s,&live.diagnostic.limits.step_s,
            &live.diagnostic.limits.ghost_s,&live.diagnostic.limits.rear_s};
        unsigned *v=limits[live.setting];
        if(x<840) {if(*v>1) (*v)--;} else if(*v<120) (*v)++;
        emit(NULL,"limits","all","changed",live.now,0);
    }
}
static VtInput input(uint64_t now,bool ctrl,bool rear,bool front) {
    VtInput s={.now=now,.buttons=live.pad.buttons,.axes={live.pad.lx,live.pad.ly,live.pad.rx,live.pad.ry},
        .rear_count=(int)live.back.reportNum,.valid=live.valid,.ctrl_new=ctrl,.rear_new=rear,.front_activity=front};
    if(live.back.reportNum) {s.rear_x=live.back.report[0].x;s.rear_y=live.back.report[0].y;}return s;
}
static void publish(void) {
    live.diagnostic.opaque=NULL;
    if(sceKernelTryLockMutex(mutex,1)==0) {published=live;sceKernelUnlockMutex(mutex,1);}
}
static int capture(SceSize size,void *arg) {
    (void)size;(void)arg;
    SceUID callback=sceKernelCreateCallback("vt_power",0,power_callback,NULL);
    callback_rc=callback<0?callback:scePowerRegisterCallback(callback);
    lifecycle_ok=callback_rc>=0;
    int setup[]={sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE),
        sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT,SCE_TOUCH_SAMPLING_STATE_START),
        sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK,SCE_TOUCH_SAMPLING_STATE_START)};
    for(int i=0;i<3;i++) if(setup[i]<0) lifecycle_ok=false;
    live.priority_ok=priorities();
    int last_quality=-99,old_rc[3]={1,1,1};
    unsigned last_write_ms=0,last_queue_ms=0;
    SceCtrlData pads[64];SceTouchData fronts[64],backs[64];
    while(!atomic_load(&finish)) {
        live.now=sceKernelGetProcessTimeWide();
        if(!session_logged && vt_logger_state()==1) {
            live.thermal_rc=scePowerGetBatteryTemp();
            if(live.thermal_rc>=0) {live.temp=live.thermal_rc;live.thermal_at=live.now;}
            live.battery=scePowerGetBatteryLifePercent();live.online=scePowerIsPowerOnline();
            for(int i=0;i<3;i++) live.clocks[i]=get_clock(NULL,i);
            emit(NULL,"session","all","begin",live.now,0);session_logged=!live.log_failed;
        }
        uint64_t gap=last_poll?live.now-last_poll:0;last_poll=live.now;
        if(gap>live.max_poll) live.max_poll=gap;
        if(gap) {unsigned bin=(unsigned)((gap+999)/1000);if(bin>101) bin=101;histogram[bin]++;polls++;}
        live.valid=true;sceKernelCheckCallback();
        SceAppMgrAppState app;memset(&app,0,sizeof(app));
        focus_rc=_sceAppMgrGetAppState(&app,sizeof(app),1);overlay=app.isSystemUiOverlaid;
        intercept=0;intercept_rc=sceCtrlGetButtonIntercept(&intercept);
        bool focused=focus_rc>=0 && intercept_rc>=0 && !overlay && !intercept;
        if(focus_rc>=0 && app.systemEventNum) {
            SceAppMgrSystemEvent ev;
            for(unsigned i=0;i<app.systemEventNum && i<16;i++) if(sceAppMgrReceiveSystemEvent(&ev)>=0)
                lifecycle(ev.systemEvent==SCE_APPMGR_SYSTEMEVENT_ON_RESUME?"app_resume":"system_event");
        }
        if(!focused || gap>50000 || scePowerIsSuspendRequired()) { if(live.running || clocks.saved) lifecycle("focus_or_poll_gap");live.valid=false; }
        if(live.restore_failed) stop_now(); // retry the saved baseline, never rebaseline failed restoration
        if(vt_logger_state()<0) {live.log_failed=true;stop_now();snprintf(live.status,sizeof(live.status),"Falha de registro: carga parada");}
        live.rc[0]=sceCtrlPeekBufferPositive(0,pads,64);
        live.rc[1]=sceTouchPeek(SCE_TOUCH_PORT_FRONT,fronts,64);
        live.rc[2]=sceTouchPeek(SCE_TOUCH_PORT_BACK,backs,64);
        int counts[3];
        for(int i=0;i<3;i++) {counts[i]=live.rc[i]>0 && live.rc[i]<=64?live.rc[i]:0;live.fresh[i]=0;}
        uint64_t newest=0;
        for(int i=0;i<counts[0];i++) if(pads[i].timeStamp>newest) newest=pads[i].timeStamp;
        for(int i=0;i<counts[1];i++) if(fronts[i].timeStamp>newest) newest=fronts[i].timeStamp;
        for(int i=0;i<counts[2];i++) if(backs[i].timeStamp>newest) newest=backs[i].timeStamp;
        // SDK buffers are chronological. Merge all three streams by device time.
        for(int i=0;i<counts[0];i++) if(pads[i].timeStamp>stamp[0]) live.fresh[0]++;
        for(int i=0;i<counts[1];i++) if(fronts[i].timeStamp>stamp[1]) live.fresh[1]++;
        for(int i=0;i<counts[2];i++) if(backs[i].timeStamp>stamp[2]) live.fresh[2]++;
        for(int i=0;i<3;i++) {
            if(live.fresh[i]) seen[i]=live.now;
            if(!counts[i] || !seen[i] || live.now-seen[i]>50000) live.valid=false;
        }
        int quality=!focused?1:(!counts[0] || !counts[1] || !counts[2])?2:!live.valid?3:0;
        if(quality!=last_quality) {
            emit(NULL,"capture_state","all",quality==0?"valid":quality==1?"system_intercepted":quality==2?"api_error":"delayed_inconclusive",live.now,0);
            last_quality=quality;
        }
        for(int i=0;i<3;i++) {
            if((live.rc[i]<0 || old_rc[i]<0) && live.rc[i]!=old_rc[i])
                emit(NULL,live.rc[i]<0?"api_error":"api_recovery",i==0?"buttons":i==1?"front":"rear","capture",live.now,0);
            old_rc[i]=live.rc[i];
        }
        int pos[3]={0};
        for(;;) {
            uint64_t ts[3]={UINT64_MAX,UINT64_MAX,UINT64_MAX};
            while(pos[0]<counts[0] && pads[pos[0]].timeStamp<=stamp[0]) pos[0]++;
            while(pos[1]<counts[1] && fronts[pos[1]].timeStamp<=stamp[1]) pos[1]++;
            while(pos[2]<counts[2] && backs[pos[2]].timeStamp<=stamp[2]) pos[2]++;
            if(pos[0]<counts[0]) ts[0]=pads[pos[0]].timeStamp;
            if(pos[1]<counts[1]) ts[1]=fronts[pos[1]].timeStamp;
            if(pos[2]<counts[2]) ts[2]=backs[pos[2]].timeStamp;
            uint64_t t=ts[0]<ts[1]?ts[0]:ts[1];if(ts[2]<t)t=ts[2];if(t==UINT64_MAX) break;
            bool front_activity=false;
            if(ts[0]==t) {live.pad=pads[pos[0]++];stamp[0]=t;}
            if(ts[1]==t) {
                SceTouchData *f=&fronts[pos[1]++];
                front_activity=f->reportNum!=live.front.reportNum || (f->reportNum && live.front.reportNum &&
                    (abs((int)f->report[0].x-live.front.report[0].x)>24 || abs((int)f->report[0].y-live.front.report[0].y)>24));
                live.front=*f;stamp[1]=t;
            }
            if(ts[2]==t) {live.back=backs[pos[2]++];stamp[2]=t;}
            if(live.front.reportNum>SCE_TOUCH_MAX_REPORT || live.back.reportNum>SCE_TOUCH_MAX_REPORT) {
                live.front.reportNum=live.back.reportNum=0;live.valid=false;
            }
            uint64_t age=newest-t,when=age<live.now?live.now-age:0;
            if(when<live.diagnostic.previous) when=live.diagnostic.previous;
            VtInput s=input(when,ts[0]==t,ts[2]==t,front_activity);
            if(age>50000) s.valid=false;
            vt_diag_feed(&live.diagnostic,&s);
        }
        VtInput latest=input(live.now,false,false,false);vt_diag_feed(&live.diagnostic,&latest);
        bool enabled=atomic_load(&stress_enabled);
        if(enabled!=stress_previous) {touch_down=true;stress_previous=enabled;}
        if(!enabled) stop_now();
        live.stress_idle=!enabled && !live.running && !clocks.saved && vt_workers_idle();
        bool was_focused=atomic_exchange(&capture_focused,live.valid);
        if(was_focused && !live.valid) atomic_fetch_add(&system_epoch,1);
        if(focused && gap<=50000 && live.fresh[1] && live.rc[1]>0) command();else touch_down=true;
        if(live.running) live.elapsed=live.now-started;
        if(!last_sample || live.now-last_sample>=VT_SECOND) {
            live.thermal_rc=scePowerGetBatteryTemp();
            if(live.thermal_rc>=0) {live.temp=live.thermal_rc;live.thermal_at=live.now;}
            live.battery=scePowerGetBatteryLifePercent();live.online=scePowerIsPowerOnline();
            for(int i=0;i<3;i++) {live.clocks[i]=get_clock(NULL,i);live.workers[i]=vt_worker_progress(i);}
            unsigned sum=0,target=polls-polls/100;
            for(int i=0;i<102;i++) {sum+=histogram[i];if(sum>=target) {live.p99_poll=(uint64_t)i*1000;break;}}
            live.priority_ok=priorities();
            if(live.running && (!live.priority_ok || live.clocks[0]!=444 || live.clocks[1]!=222 || live.clocks[2]!=222)) {
                stop_now();emit(NULL,"stress","all","priority_or_clock_changed",live.now,0);
            }
            emit(NULL,"sample","all","telemetry",live.now,0);last_sample=live.now;
        }
        vt_log_status(&live.confirmed,&live.write_ms,&live.queue_delay_ms);
        if((live.write_ms>50 && live.write_ms>last_write_ms) || (live.queue_delay_ms>50 && live.queue_delay_ms>last_queue_ms))
            emit(NULL,"logger_delay","disk","new_maximum",live.now,0);
        last_write_ms=live.write_ms;last_queue_ms=live.queue_delay_ms;publish();
        uint64_t spent=sceKernelGetProcessTimeWide()-live.now;
        sceKernelDelayThreadCB(spent<8000?(unsigned)(8000-spent):1000);
    }
    live.now=sceKernelGetProcessTimeWide();stop_now();emit(NULL,"session","all","end",live.now,0);publish();
    if(callback>=0) {scePowerUnregisterCallback(callback);sceKernelDeleteCallback(callback);}return 0;
}
static const char *init_stage="not_started";
const char *vt_runtime_init_stage(void) {return init_stage;}
int vt_runtime_init(void) {
    init_stage="ui_priority";
    ui_thread=sceKernelGetThreadId();
    int rc=sceKernelChangeThreadPriority(ui_thread,VT_UI_PRIORITY);
    if(rc<0) return rc;
    clocks.get=get_clock;clocks.set=set_clock;
    live.pad.lx=live.pad.ly=live.pad.rx=live.pad.ry=128;live.temp=-1;
    vt_diag_init(&live.diagnostic,emit,NULL);
    snprintf(session,sizeof(session),"%lld-%llu",(long long)time(NULL),(unsigned long long)sceKernelGetProcessTimeWide());
    snprintf(live.status,sizeof(live.status),"VitaTester 1.5.1 - stress desligado");published=live;
    init_stage="snapshot_mutex";
    mutex=sceKernelCreateMutex("vt_snapshot",0,0,NULL);if(mutex<0) return mutex;
    vt_logger_start();vt_workers_start();
    init_stage="capture_create";
    thread=sceKernelCreateThread("vt_capture",capture,VT_CAPTURE_PRIORITY,65536,0,SCE_KERNEL_CPU_MASK_USER_ALL,NULL);
    if(thread<0) return thread;
    init_stage="capture_start";
    rc=sceKernelStartThread(thread,0,NULL);if(rc<0) return rc;
    init_stage="ready";
    return 0;
}
void vt_runtime_snapshot(VtSnapshot *out) {
    if(mutex>=0 && sceKernelTryLockMutex(mutex,1)==0) {*out=published;sceKernelUnlockMutex(mutex,1);}
}
void vt_runtime_fps(unsigned n) {atomic_store(&fps,n);}
void vt_runtime_shutdown(void) {
    atomic_store(&finish,true);
    if(thread>=0) {SceUInt timeout=1000000;sceKernelWaitThreadEnd(thread,NULL,&timeout);}
    vt_workers_finish();vt_logger_finish();
}
