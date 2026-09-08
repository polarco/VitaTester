#include "runtime.h"
#include "queue.h"
#include "safety.h"
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <string.h>
static VtQueue queue;
static atomic_int state; // 0 opening, 1 ready, -1 terminal failure
static atomic_bool finish;
static atomic_uint confirmed,write_ms,delay_ms;
static SceUID thread=-1,fd=-1;
static int write_file(void *ctx,const void *buf,unsigned len) { (void)ctx;return sceIoWrite(fd,buf,len); }
static int sync_file(void *ctx) { (void)ctx;return sceIoSyncByFd(fd,0); }
static int logger(SceSize args,void *argp) {
    (void)args;(void)argp;
    sceIoMkdir("ux0:data",0777); sceIoMkdir("ux0:data/VitaTester",0777);
    fd=sceIoOpen("ux0:data/VitaTester/stresslog.txt",SCE_O_WRONLY|SCE_O_CREAT|SCE_O_APPEND,0666);
    if(fd<0) {atomic_store(&state,-1);return 0;}
    // A newline isolates any torn trailing record from an earlier forced shutdown.
    if(!vt_write_record(write_file,sync_file,0,"\n",1)) {atomic_store(&state,-1);sceIoClose(fd);return 0;}
    atomic_store(&state,1);
    VtRecord record;
    for(;;) {
        if(vt_queue_pop(&queue,&record)) {
            uint64_t begin=sceKernelGetProcessTimeWide();
            unsigned delay=(unsigned)((begin-record.queued)/1000);
            if(delay>atomic_load(&delay_ms)) atomic_store(&delay_ms,delay);
            if(!vt_write_record(write_file,sync_file,0,record.text,record.len)) {atomic_store(&state,-1);break;}
            unsigned duration=(unsigned)((sceKernelGetProcessTimeWide()-begin)/1000);
            if(duration>atomic_load(&write_ms)) atomic_store(&write_ms,duration);
            atomic_store(&confirmed,record.seq);
        } else if(atomic_load(&finish)) break;
        else sceKernelDelayThread(2000);
    }
    sceIoClose(fd);return 0;
}
int vt_logger_start(void) {
    thread=sceKernelCreateThread("vt_logger",logger,VT_LOG_PRIORITY,32768,0,0x7,NULL);
    if(thread<0 || sceKernelStartThread(thread,0,NULL)<0) {atomic_store(&state,-1);return -1;}
    return 0;
}
SceUID vt_logger_id(void) {return thread;}
int vt_logger_state(void) {return atomic_load(&state);}
bool vt_log_enqueue(const char *text,unsigned len,unsigned seq,uint64_t now) {
    if(atomic_load(&state)!=1 || len>=VT_RECORD_SIZE) return false;
    VtRecord r={.len=len,.seq=seq,.queued=now};memcpy(r.text,text,len);
    if(!vt_queue_push(&queue,&r)) {atomic_store(&state,-1);return false;}return true;
}
void vt_log_status(unsigned *seq,unsigned *write,unsigned *delay) {
    *seq=atomic_load(&confirmed);*write=atomic_load(&write_ms);*delay=atomic_load(&delay_ms);
}
void vt_logger_finish(void) {
    atomic_store(&finish,true);
    if(thread>=0) {SceUInt timeout=2000000;sceKernelWaitThreadEnd(thread,NULL,&timeout);}
}
