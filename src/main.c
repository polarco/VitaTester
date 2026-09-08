#include "module.h"
#include "navigation.h"
#include <psp2/kernel/processmgr.h>
#include <psp2/display.h>
extern unsigned int basicfont_size;
extern unsigned char basicfont[];
int main(void) {
    vita2d_init();vita2d_set_clear_color(RGBA8(0,0,0,255));
    if(vt_runtime_init()<0) {vt_runtime_shutdown();sceKernelExitProcess(1);return 1;}
    vita2d_font *font=vita2d_load_font_mem(basicfont,basicfont_size);
    vt_original_init();
    const VtModule *active=NULL;
    VtSnapshot s={0};VtGesture gesture={.latched=true};
    bool exiting=false,down=true;unsigned epoch=vt_runtime_epoch(),frames=0,fps=0;
    uint64_t fps_at=sceKernelGetProcessTimeWide();
    for(;;) {
        vt_runtime_snapshot(&s);
        uint64_t now=sceKernelGetProcessTimeWide();
        if(++frames && now-fps_at>=1000000) {fps=frames*1000000ULL/(now-fps_at);frames=0;fps_at=now;vt_runtime_fps(fps);}
        unsigned current=vt_runtime_epoch();
        bool valid=s.valid && now>=s.now && now-s.now<=50000;
        if(current!=epoch || !valid) {
            if(active && active->system_event) active->system_event();
            epoch=current;down=true;gesture.latched=true;gesture.since=0;
        }
        unsigned central=0;
        for(unsigned i=0;i<s.front.reportNum;i++) {
            int x=s.front.report[i].x/2,y=s.front.report[i].y/2;
            if(x>=240 && x<720 && y>=110 && y<420) central++;
        }
        if(vt_gesture(&gesture,now,valid,s.front.reportNum,central) && active) exiting=true;
        if(exiting && active && (!active->leave || active->leave(&s))) {active=NULL;exiting=false;down=true;}
        if(valid && !s.front.reportNum) down=false;
        if(valid && s.front.reportNum==1 && !down) {
            down=true;
            int x=s.front.report[0].x/2,y=s.front.report[0].y/2;
            if(active) {if(active->input) active->input(x,y);}
            else if(x>=210 && x<750 && y>=150 && y<150+(int)vt_module_count*80) {
                active=vt_modules[(y-150)/80];if(active->enter) active->enter();
            }
        }
        if(active && active->update) active->update(&s);
        vita2d_start_drawing();vita2d_clear_screen();
        if(active) active->render(font,&s,fps);
        else {
            vita2d_font_draw_text(font,210,80,0xffffffff,36,"VitaTester 1.5.0");
            for(unsigned i=0;i<vt_module_count;i++) {
                vita2d_draw_rectangle(210,150+i*80,540,65,RGBA8(32,53,72,255));
                vita2d_font_draw_text(font,235,192+i*80,0xffffffff,27,vt_modules[i]->name);
            }
            vita2d_font_draw_text(font,70,440,0xffffffff,21,"Menu: mantenha 3 dedos no centro por 2 segundos.");
            vita2d_font_draw_text(font,70,473,0xffffffff,19,"Solte os dedos antes de repetir. Start/Select: somente diagnostico.");
            vita2d_font_draw_text(font,70,510,0xffffffff,19,"PS: LiveArea. Para fechar, encerre a bolha.");
        }
        if(exiting) {
            vita2d_draw_rectangle(0,0,960,38,RGBA8(80,20,20,255));
            vita2d_font_draw_text(font,12,27,0xffffffff,20,s.restore_failed?"Falha ao restaurar clocks: troca bloqueada; tentando novamente":"Encerrando modulo; em erro, consulte o resultado e toque Salvar.");
        }
        vita2d_end_drawing();vita2d_swap_buffers();sceDisplayWaitVblankStart();
    }
}
