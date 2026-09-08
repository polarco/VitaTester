#include "module.h"
static void enter(void) {vt_runtime_stress(true);}
static void render(vita2d_font *f,const VtSnapshot *s,unsigned fps) {vt_original_draw(f,s,fps,true);}
static bool leave(const VtSnapshot *s) {vt_runtime_stress(false);return s->stress_idle && !s->running && !s->restore_failed;}
const VtModule vt_module_stress={.id="stress",.name="Stress Test",.enter=enter,.render=render,.leave=leave};
