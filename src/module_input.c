#include "module.h"
static void render(vita2d_font *f,const VtSnapshot *s,unsigned fps) {vt_original_draw(f,s,fps,false);}
const VtModule vt_module_input={.id="input",.name="Input Test",.render=render};
