#include "module.h"
#include "navigation.h"
#include <psp2/kernel/processmgr.h>
#include <psp2/display.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <stdio.h>
#include <string.h>
extern unsigned int basicfont_size;
extern unsigned char basicfont[];
/* Append only stage/status data; no device identifiers or raw input. */
static void boot_record(const char *stage,int rc) {
    sceIoMkdir("ux0:data",0777);sceIoMkdir("ux0:data/VitaTester",0777);
    SceUID fd=sceIoOpen("ux0:data/VitaTester/startup.txt",SCE_O_WRONLY|SCE_O_CREAT|SCE_O_APPEND,0666);
    if(fd<0)return;
    char line[160];int n=snprintf(line,sizeof(line),"1.5.2 %s 0x%08X\n",stage,(unsigned)rc);
    if(n>0 && n<(int)sizeof(line)) {
        int off=0;while(off<n){int written=sceIoWrite(fd,line+off,n-off);if(written<=0)break;off+=written;}
        sceIoSyncByFd(fd,0);
    }
    sceIoClose(fd);
}
static void init_error(vita2d_font *font,const char *stage,int rc) {
    for(;;) {
        vita2d_start_drawing();vita2d_clear_screen();
        vita2d_font_draw_text(font,60,120,0xffffffff,30,"VitaTester 1.5.2 - falha ao iniciar");
        vita2d_font_draw_textf(font,60,200,0xffffffff,24,"Etapa: %s | erro: 0x%08X",stage,(unsigned)rc);
        vita2d_font_draw_text(font,60,290,0xffffffff,22,"Informe esta etapa e codigo. Nenhuma carga foi iniciada.");
        vita2d_font_draw_text(font,60,360,0xffffffff,22,"PS: LiveArea. Para fechar, encerre a bolha.");
        vita2d_end_drawing();vita2d_swap_buffers();sceDisplayWaitVblankStart();
    }
}
int main(void) {
    boot_record("main",0);
    int rc=vita2d_init();boot_record("graphics",rc);
    if(rc<0){sceKernelExitProcess(1);return 1;}
    vita2d_set_clear_color(RGBA8(0,0,0,255));
    vita2d_font *font=vita2d_load_font_mem(basicfont,basicfont_size);
    boot_record("font",font?0:-1);
    if(!font){sceKernelExitProcess(1);return 1;}
    rc=vt_runtime_init();boot_record(vt_runtime_init_stage(),rc);
    if(rc<0) {vt_runtime_shutdown();init_error(font,vt_runtime_init_stage(),rc);return 1;}
    vt_original_init();
    boot_record("menu",0);
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
            vita2d_font_draw_text(font,210,80,0xffffffff,36,"VitaTester 1.5.2");
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
