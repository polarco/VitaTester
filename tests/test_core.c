#include "diagnostic.h"
#include "safety.h"
#include "queue.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static unsigned held,inactive,recover,warning,ghost,missing,results,observed,quality;
static void emit(void *ctx,const char *ev,const char *input,const char *state,uint64_t now,uint64_t good) {
    (void)ctx;(void)input;assert(good<=now);
    if(!strcmp(ev,"suspect")) {if(!strcmp(state,"held"))held++;if(!strcmp(state,"inactive"))inactive++;if(!strcmp(state,"ghost"))ghost++;if(!strcmp(state,"guided_touch_missing"))missing++;}
    if(!strcmp(ev,"recovery"))recover++;
    if(!strcmp(ev,"warning"))warning++;
    if(!strcmp(ev,"guided_result")) {results++;if(!strcmp(state,"observed"))observed++;}
    if(!strcmp(ev,"capture_quality"))quality++;
}
static VtDiagnostic d;
static VtInput s;
static void reset(void) {
    held=inactive=recover=warning=ghost=missing=results=observed=quality=0;
    vt_diag_init(&d,emit,NULL);s=(VtInput){.now=1000000,.axes={128,128,128,128},.valid=true,.ctrl_new=true,.rear_new=true};vt_diag_feed(&d,&s);
}
static void tick(void) {s.now+=10000;vt_diag_feed(&d,&s);}
static void seconds(unsigned n) {for(unsigned i=0;i<n*100;i++)tick();}
static void diagnostic_tests(void) {
    reset();seconds(90);assert(!held && !inactive && !warning); // legitimate unused input
    s.buttons=vt_masks[0];tick();seconds(9);assert(!held);seconds(1);assert(held==1);
    seconds(20);assert(held==1);s.buttons=0;tick();assert(recover==1);
    reset();s.buttons=vt_masks[0];tick();s.buttons=0;tick();seconds(31);assert(!inactive);
    s.buttons=vt_masks[1];tick();assert(inactive==1);s.buttons|=vt_masks[0];tick();assert(recover==1);
    reset();s.buttons=vt_masks[0];tick();s.buttons=0;tick();
    for(int i=0;i<4000;i++) {s.axes[0]=127+i%3;tick();}assert(!inactive); // analog noise
    s.axes[0]=190;tick();assert(inactive==1);
    reset();s.buttons=vt_masks[0];tick();seconds(9);s.valid=false;seconds(20);assert(!held);
    s.valid=true;seconds(9);assert(!held);seconds(2);assert(held==1);
    reset();s.buttons=vt_masks[0];tick();seconds(9);s.now+=50001;vt_diag_feed(&d,&s);seconds(9);assert(!held);seconds(2);assert(held==1);
    reset();s.rear_count=1;seconds(9);assert(!warning);seconds(1);assert(!warning);tick();assert(warning==1);seconds(1);assert(warning==1);s.rear_count=0;tick();assert(recover==1);
    reset();vt_diag_mode(&d,true,s.now);seconds(160);assert(results==16 && missing==1 && !ghost); // no touch is suspect only on request
    reset();vt_diag_mode(&d,true,s.now);s.buttons=vt_masks[0];tick();s.buttons=0;tick();seconds(10);assert(observed==1 && d.step==1);
    // No controller sample must not index a negative analog axis in the digital stage.
    s.ctrl_new=false;seconds(2);assert(d.step==1);
    reset();vt_diag_mode(&d,true,s.now);d.step=14;s.rear_count=1;tick();s.rear_x=100;tick();seconds(10);assert(observed==1 && d.step==15);
    seconds(1);assert(!ghost);seconds(1);assert(ghost==1);s.rear_count=0;tick();assert(recover==1);seconds(8);assert(observed==1); // recovery doesn't turn failed no-touch into pass
    reset();vt_diag_mode(&d,true,s.now);seconds(9);s.valid=false;seconds(30);assert(!results);s.valid=true;seconds(9);assert(!results);seconds(2);assert(results==1);
    reset();vt_diag_mode(&d,true,s.now);d.step=14;s.rear_count=1;seconds(10);assert(!missing && observed==0); // touch seen, missing slide isn't missing touch
    reset();vt_diag_mode(&d,true,s.now);d.step=14;seconds(10);assert(missing==1);s.rear_count=1;tick();assert(recover==1);
    reset();vt_diag_mode(&d,true,s.now);d.step=14;s.rear_count=1;
    for(int i=0;i<1000;i++) {s.rear_x=i/2;tick();}assert(observed==1); // slow cumulative slide
    puts("diagnostics: time boundaries, inactivity, noise, guided, recovery, focus/gaps OK");
}
typedef struct {int values[3],fail,read_bad,calls,fail_restore;} Fake;
static int get(void *v,int i) {Fake *f=v;return f->read_bad==i?-1:f->values[i];}
static int set(void *v,int i,int n) {Fake *f=v;f->calls++;if((f->fail==i && n==(i==0?444:222)) || (f->fail_restore && n==111)) return -1;f->values[i]=n;return 0;}
static void clocks_tests(void) {
    for(int fail=-1;fail<3;fail++) {
        Fake f={.values={333,111,166},.fail=fail,.read_bad=-1};VtClocks c={.get=get,.set=set,.opaque=&f};
        assert(vt_clocks_start(&c)==(fail==-1));
        assert(vt_clocks_stop(&c));int calls=f.calls;assert(vt_clocks_stop(&c) && f.calls==calls);
        assert(f.values[0]==333 && f.values[1]==111 && f.values[2]==166);
    }
    Fake f={.values={333,111,166},.fail=-1,.read_bad=1};VtClocks c={.get=get,.set=set,.opaque=&f};assert(!vt_clocks_start(&c) && f.calls==0);
    f.read_bad=-1;assert(vt_clocks_start(&c));f.fail_restore=1;assert(!vt_clocks_stop(&c) && c.saved && !c.running);
    assert(!vt_clocks_start(&c));f.fail_restore=0;assert(vt_clocks_stop(&c) && !c.saved);assert(f.values[1]==111);
    puts("clocks: partial setters, invalid baseline, restoration retry, idempotence OK");
}
typedef struct {char output[32];unsigned used,chunk,calls;int fail,sync_fail,synced;} Writer;
static int write_(void *v,const void *p,unsigned n) {Writer *w=v;if(w->fail && ++w->calls==2)return 0;if(n>w->chunk)n=w->chunk;memcpy(w->output+w->used,p,n);w->used+=n;return (int)n;}
static int sync_(void *v) {Writer *w=v;w->synced++;return w->sync_fail?-1:0;}
static void storage_tests(void) {
    Writer w={.chunk=2};assert(vt_write_record(write_,sync_,&w,"example\n",8));assert(w.used==8 && w.synced==1 && !memcmp(w.output,"example\n",8));
    w=(Writer){.chunk=2,.fail=1};assert(!vt_write_record(write_,sync_,&w,"example\n",8) && w.synced==0);
    w=(Writer){.chunk=2,.sync_fail=1};assert(!vt_write_record(write_,sync_,&w,"example\n",8));
    VtQueue *q=calloc(1,sizeof(*q));assert(q);VtRecord a={0},b;
    for(unsigned round=0;round<3;round++) {
        for(unsigned i=0;i<VT_QUEUE_CAP;i++) {a.seq=i;assert(vt_queue_push(q,&a));}
        assert(!vt_queue_push(q,&a));
        for(unsigned i=0;i<VT_QUEUE_CAP;i++) {assert(vt_queue_pop(q,&b));assert(b.seq==i);}
        assert(!vt_queue_pop(q,&b));
    }free(q);puts("storage: full writes, zero write, sync failure, saturated queue and wrap OK");
}
int main(void) {diagnostic_tests();clocks_tests();storage_tests();return 0;}
