#include "navigation.h"
bool vt_gesture(VtGesture *g,uint64_t now,bool valid,unsigned total,unsigned central) {
    if(!valid || (g->last && (now<g->last || now-g->last>50000))) {g->since=0;g->latched=true;}
    g->last=now;
    if(!valid) return false;
    if(!total) {g->latched=false;g->since=0;return false;}
    if(g->latched) return false;
    if(total!=3 || central!=3) {g->since=0;return false;}
    if(!g->since) g->since=now;
    if(now-g->since<2000000) return false;
    g->latched=true;g->since=0;return true;
}
