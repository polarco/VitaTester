#include "diagnostic.h"
#include <stdlib.h>
#include <string.h>
const uint32_t vt_masks[VT_BUTTONS] = {0x10,0x40,0x80,0x20,0x4000,0x2000,0x8000,0x1000,0x100,0x200,0x8,0x1};
const char *vt_names[VT_BUTTONS] = {"UP","DOWN","LEFT","RIGHT","CROSS","CIRCLE","SQUARE","TRIANGLE","L","R","START","SELECT"};
static void event(VtDiagnostic *d,const char *ev,const char *in,const char *state,uint64_t now,uint64_t good) {
    if(d->emit) d->emit(d->opaque,ev,in,state,now,good);
}
void vt_diag_init(VtDiagnostic *d,VtEmit emit,void *opaque) {
    memset(d,0,sizeof(*d)); d->limits=(VtLimits){10,30,10,2,10}; d->emit=emit; d->opaque=opaque;
    for(int i=0;i<4;i++) d->axes[i]=128;
}
void vt_diag_mode(VtDiagnostic *d,bool guided,uint64_t now) {
    d->guided=guided; d->step=0; d->step_time=0; d->step_pressed=false; d->step_success=false; d->step_contact=false;
    d->ghost_time=0; d->ghost_alert=false;
    event(d,"mode","all",guided?"guided":"free",now,0);
}
const char *vt_step_name(const VtDiagnostic *d) {
    if(!d->guided) return "Livre: ausencia espontanea nao comprova defeito";
    if(d->step<VT_BUTTONS) return vt_names[d->step];
    const char *extra[]={"Mova analogico esquerdo","Mova analogico direito","Toque e deslize atras","Retire os dedos de tras","Roteiro concluido"};
    return extra[d->step-VT_BUTTONS];
}
void vt_diag_feed(VtDiagnostic *d,const VtInput *s) {
    uint64_t dt=d->initialized && s->now>=d->previous?s->now-d->previous:0;
    bool valid=s->valid && dt<=50000;
    if(!d->initialized || valid!=d->valid)
        event(d,"capture_quality","all",valid?"valid":"inconclusive",s->now,0);
    bool baseline=!d->initialized || !d->valid || !valid;
    d->previous=s->now; d->initialized=true; d->valid=valid;
    if(baseline) {
        for(int i=0;i<VT_BUTTONS;i++) {
            d->button[i].down=(s->buttons & vt_masks[i])!=0;
            d->button[i].held=0; d->button[i].idle=0; d->independent[i]=0;
            d->button[i].pressed=false;
        }
        memcpy(d->axes,s->axes,sizeof(d->axes));
        d->rear_time=0; d->ghost_time=0;
        d->rear_count=s->rear_count; d->rear_x=s->rear_x; d->rear_y=s->rear_y;
        // Restart the current guided stage; a transition across a gap proves nothing.
        d->step_time=0; d->step_pressed=false; d->step_success=false; d->step_contact=false;
        return;
    }
    bool analog=false;
    if(s->ctrl_new) for(int a=0;a<4;a++) {
        if(abs(s->axes[a]-128)>20 && abs(s->axes[a]-d->axes[a])>=12) analog=true;
    }
    bool rear_activity=s->rear_new && (s->rear_count!=d->rear_count ||
        (s->rear_count && (abs(s->rear_x-d->rear_x)>=24 || abs(s->rear_y-d->rear_y)>=24)));
    uint32_t changed=0;
    if(s->ctrl_new) for(int i=0;i<VT_BUTTONS;i++)
        if(d->button[i].down!=((s->buttons & vt_masks[i])!=0)) changed|=vt_masks[i];
    for(int i=0;i<VT_BUTTONS;i++) {
        VtButton *b=&d->button[i];
        if(analog || rear_activity || s->front_activity || (changed & ~vt_masks[i])) d->independent[i]=s->now;
        if(s->ctrl_new && (changed & vt_masks[i])) {
            b->down=(s->buttons & vt_masks[i])!=0;
            if(b->down) b->pressed=true;
            else if(b->pressed) { b->cycled=true; b->last_good=s->now; }
            b->held=0; b->idle=0;
            event(d,"button",vt_names[i],b->down?"pressed":"released",s->now,b->last_good);
            if(b->held_alert) {event(d,"recovery",vt_names[i],"held",s->now,b->last_good);b->held_alert=false;}
            if(b->idle_alert) {event(d,"recovery",vt_names[i],"inactive",s->now,b->last_good);b->idle_alert=false;}
        }
        if(!(changed & vt_masks[i])) {
            b->idle+=dt;
            if(b->down) b->held+=dt;
        }
        if(b->down && b->held>=d->limits.held_s*VT_SECOND && !b->held_alert) {
            b->held_alert=true; event(d,"suspect",vt_names[i],"held",s->now,b->last_good);
        }
        if(b->cycled && b->idle>=d->limits.idle_s*VT_SECOND && d->independent[i] &&
           s->now-d->independent[i]<=5*VT_SECOND && !b->idle_alert) {
            b->idle_alert=true; event(d,"suspect",vt_names[i],"inactive",s->now,b->last_good);
        }
    }
    if(s->rear_count && d->missing_alert) {
        event(d,"recovery","rear","guided_touch_missing",s->now,s->now);d->missing_alert=false;
    }
    if(s->rear_count) {
        if(s->rear_new && !d->rear_count) {d->rear_time=0;d->rear_good=s->now;}
        else d->rear_time+=dt;
    }
    else {
        d->rear_time=0; d->rear_good=s->now;
        if(d->rear_alert) {event(d,"recovery","rear","persistent",s->now,d->rear_good); d->rear_alert=false;}
        if(d->ghost_alert) {event(d,"recovery","rear","ghost",s->now,d->rear_good); d->ghost_alert=false;}
    }
    if(!d->guided && d->rear_time>=d->limits.rear_s*VT_SECOND && !d->rear_alert) {
        d->rear_alert=true; event(d,"warning","rear","persistent",s->now,d->rear_good);
    }
    if(d->guided && d->step<VT_BUTTONS+4) {
        d->step_time+=dt;
        if(d->step<VT_BUTTONS) {
            if(s->ctrl_new && (changed & vt_masks[d->step])) {
                if(s->buttons & vt_masks[d->step]) d->step_pressed=true;
                else if(d->step_pressed) d->step_success=true;
            }
        } else if(d->step<VT_BUTTONS+2) {
            int a=(d->step-VT_BUTTONS)*2;
            if(s->ctrl_new && (abs(s->axes[a]-128)>60 || abs(s->axes[a+1]-128)>60)) d->step_success=true;
        } else if(d->step==VT_BUTTONS+2 && s->rear_new) {
            if(s->rear_count) d->step_contact=true;
            if(s->rear_count && d->rear_count && (abs(s->rear_x-d->rear_x)>=24 || abs(s->rear_y-d->rear_y)>=24)) d->step_success=true;
        } else if(d->step==VT_BUTTONS+3) {
            if(s->rear_count) d->ghost_time+=dt; else d->ghost_time=0;
            if(d->ghost_time>=d->limits.ghost_s*VT_SECOND && !d->ghost_alert) {
                d->ghost_alert=true; event(d,"suspect","rear","ghost",s->now,d->rear_good);
                d->step_pressed=true; // remember failure even after recovery
            }
            d->step_success=!d->step_pressed && s->rear_count==0;
        }
        if(d->step_time>=d->limits.step_s*VT_SECOND) {
            const char *name=vt_step_name(d);
            event(d,"guided_result",name,d->step_success?"observed":"not_observed",s->now,0);
            if(d->step==VT_BUTTONS+2 && !d->step_contact && !d->missing_alert) {
                d->missing_alert=true;
                event(d,"suspect","rear","guided_touch_missing",s->now,d->rear_good);
            }
            d->step++; d->step_time=0; d->step_pressed=false; d->step_success=false; d->step_contact=false; d->ghost_time=0;
        }
    }
    if(s->ctrl_new && analog) memcpy(d->axes,s->axes,sizeof(d->axes));
    if(s->rear_new && rear_activity) { d->rear_count=s->rear_count; d->rear_x=s->rear_x; d->rear_y=s->rear_y;
        if(s->rear_count) d->rear_good=s->now;
    }
}
