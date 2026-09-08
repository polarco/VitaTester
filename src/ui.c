#include "ui.h"
#include <stdio.h>
#define WHITE RGBA8(240,244,248,255)
#define YELLOW RGBA8(255,211,70,255)
#define RED RGBA8(255,100,100,255)
#define GREEN RGBA8(95,240,150,255)
void vt_draw_hud(vita2d_font *font,const VtSnapshot *s,unsigned fps) {
    vita2d_draw_rectangle(0,0,960,33,RGBA8(8,12,18,255));
    vita2d_font_draw_text(font,12,24,WHITE,19,"PS: voltar à LiveArea • Para fechar, encerre a bolha");
    vita2d_draw_rectangle(220,96,520,334,RGBA8(8,12,18,240));
    unsigned color=s->log_failed || s->restore_failed?RED:s->running?GREEN:WHITE;
    vita2d_font_draw_textf(font,234,121,color,20,"VitaTester 1.5.0 | %s | %llus",s->running?"STRESS":"PARADO",(unsigned long long)(s->elapsed/VT_SECOND));
    if(s->temp>=0 && s->thermal_rc>=0) vita2d_font_draw_textf(font,234,146,WHITE,18,"Bateria: %.2f °C | %d%% | AC: %s",s->temp/100.0,s->battery,s->online>0?"sim":"nao");
    else vita2d_font_draw_text(font,234,146,RED,18,"Temperatura da bateria indisponivel");
    vita2d_font_draw_textf(font,234,170,WHITE,17,"ARM/GPU/BUS: %d/%d/%d MHz | FPS %u",s->clocks[0],s->clocks[1],s->clocks[2],fps);
    vita2d_font_draw_textf(font,234,194,s->valid?WHITE:RED,17,"Poll max %.1f ms | p99 %s%.1f ms | %s",s->max_poll/1000.0,s->p99_poll>=101000?">=":"",s->p99_poll/1000.0,s->valid?"valido":"INCONCLUSIVO");
    vita2d_font_draw_textf(font,234,218,s->priority_ok?WHITE:RED,16,"Workers %u / %u / %u | prioridades %s",s->workers[0],s->workers[1],s->workers[2],s->priority_ok?"OK":"bloqueadas");
    vita2d_font_draw_textf(font,234,241,s->log_failed?RED:WHITE,16,"Log #%u | max escrita/fila %u/%u ms",s->confirmed,s->write_ms,s->queue_delay_ms);
    if(s->diagnostic.guided && s->diagnostic.step<VT_BUTTONS)
        vita2d_font_draw_textf(font,234,265,WHITE,18,"Pressione e solte: %s",vt_step_name(&s->diagnostic));
    else vita2d_font_draw_text(font,234,265,WHITE,16,vt_step_name(&s->diagnostic));
    if(s->diagnostic.guided) vita2d_font_draw_textf(font,234,287,WHITE,16,"Etapa %d/16 | %llu/%u s validos",s->diagnostic.step<16?s->diagnostic.step+1:16,(unsigned long long)(s->diagnostic.step_time/VT_SECOND),s->diagnostic.limits.step_s);
    int row=0;
    for(int i=0;i<VT_BUTTONS;i++) if(s->diagnostic.button[i].held_alert || s->diagnostic.button[i].idle_alert) {
        vita2d_font_draw_textf(font,234+(row%3)*163,309+(row/3)*19,YELLOW,15,"%s: %s",vt_names[i],s->diagnostic.button[i].held_alert?"mantido":"inativo");row++;
    }
    if(s->diagnostic.rear_alert || s->diagnostic.ghost_alert || s->diagnostic.missing_alert) vita2d_font_draw_text(font,234,392,YELLOW,16,"Traseiro: suspeita / persistencia (veja log)");
    vita2d_font_draw_text(font,234,416,s->restore_failed || s->log_failed?RED:WHITE,14,s->restore_failed?"FALHA RESTAURACAO: tentando baseline novamente":s->status);
    const char *names[]={"Mantido","Inativo","Etapa","Fantasma","Persistencia"};
    unsigned values[]={s->diagnostic.limits.held_s,s->diagnostic.limits.idle_s,s->diagnostic.limits.step_s,s->diagnostic.limits.ghost_s,s->diagnostic.limits.rear_s};
    const int xs[]={0,240,480,720,840},ws[]={238,238,238,118,118};
    for(int i=0;i<5;i++) vita2d_draw_rectangle(xs[i],456,ws[i],45,RGBA8(32,53,72,255));
    vita2d_font_draw_text(font,30,485,WHITE,20,s->running?"Parar stress":"Iniciar stress");
    vita2d_font_draw_text(font,271,485,WHITE,20,s->diagnostic.guided?"Modo livre":"Modo guiado");
    vita2d_font_draw_textf(font,495,484,WHITE,18,"%s: %us",names[s->setting],values[s->setting]);
    vita2d_font_draw_text(font,767,486,WHITE,24,"-");vita2d_font_draw_text(font,887,486,WHITE,24,"+");
}
