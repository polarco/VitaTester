#include "safety.h"
bool vt_clocks_stop(VtClocks *c) {
    c->running=false;
    if(!c->saved) return true;
    bool ok=true;
    for(int i=0;i<3;i++) c->restore_rc[i]=c->set(c->opaque,i,c->baseline[i]);
    for(int i=0;i<3;i++) {
        c->effective[i]=c->get(c->opaque,i);
        if(c->restore_rc[i]<0 || c->effective[i]!=c->baseline[i]) ok=false;
    }
    if(ok) c->saved=false;
    return ok;
}
bool vt_clocks_start(VtClocks *c) {
    const int target[3]={444,222,222};
    if(c->running) return true;
    if(c->saved && !vt_clocks_stop(c)) return false;
    for(int i=0;i<3;i++) {c->baseline[i]=c->get(c->opaque,i);if(c->baseline[i]<=0) return false;}
    c->saved=true;
    bool ok=true;
    for(int i=0;i<3;i++) {c->setter_rc[i]=c->set(c->opaque,i,target[i]);if(c->setter_rc[i]<0) ok=false;}
    for(int i=0;i<3;i++) {c->effective[i]=c->get(c->opaque,i);if(c->effective[i]!=target[i]) ok=false;}
    if(!ok) {vt_clocks_stop(c);return false;}
    c->running=true;return true;
}
bool vt_write_record(VtWrite write,VtSync sync,void *ctx,const char *data,size_t len) {
    size_t done=0;
    while(done<len) {
        int n=write(ctx,data+done,(unsigned)(len-done));
        if(n<=0 || (size_t)n>len-done) return false;
        done+=(size_t)n;
    }
    return sync(ctx)>=0;
}
