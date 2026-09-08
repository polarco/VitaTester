#ifndef VT_MODULE_H
#define VT_MODULE_H
#include "runtime.h"
#include <vita2d.h>
typedef struct {
 const char *id, *name;
 void (*enter)(void);
 void (*input)(int x,int y);
 void (*update)(const VtSnapshot *);
 void (*render)(vita2d_font *,const VtSnapshot *,unsigned);
 void (*system_event)(void);
 bool (*leave)(const VtSnapshot *);
} VtModule;
extern const VtModule *const vt_modules[];
extern const unsigned vt_module_count;
void vt_original_init(void);
void vt_original_draw(vita2d_font *,const VtSnapshot *,unsigned,bool);
void vt_original_fini(void);
#endif
