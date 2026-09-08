#include "module.h"
#include "scanner.h"
#include <stdio.h>
#include <string.h>
static VtScannerView view;
static int selected,page,scroll,file_page,init_error;
static char chosen[512];
static vita2d_texture *preview;
static unsigned preview_frame;
static const char *instructions[]={
 "Incline lentamente o aparelho em cada eixo; observe os vetores.",
 "Gire lentamente o aparelho nos tres eixos; observe a velocidade.",
 "Mova longe de metais; observe NED e estabilidade: 0 instavel, 1 sem uso, 2 estavel.",
 "Desconecte fones/saidas externas. Ouca 440 Hz por 3 s no alto-falante esquerdo.",
 "Desconecte fones/saidas externas. Ouca 440 Hz por 3 s no alto-falante direito.",
 "Fale perto do microfone. Observe RMS/pico por ate 10 s; audio nao e salvo.",
 "Aponte a camera frontal para uma cena iluminada. Preview por ate 30 s.",
 "Aponte a camera traseira para uma cena iluminada. Preview por ate 30 s.",
 "Conecte pelo sistema antes de iniciar. Estado/sinal somente; busca AP indisponivel.",
 "Bluetooth: API userland documentada indisponivel. Inicie e confirme PULADO.",
 "Toque Arquivo, escolha montagem e arquivo existente; ate 1 MiB, somente leitura."
};
static void button(vita2d_font *f,int x,int y,int w,const char *text) {
 vita2d_draw_rectangle(x,y,w-3,39,RGBA8(32,53,72,255));vita2d_font_draw_text(f,x+9,y+26,0xffffffff,18,text);
}
static void lines(vita2d_font *f,const char *text,int x,int y,int width,int rows,int skip) {
 const char *p=text;int row=0,index=0;
 while(*p && row<rows) {
  char line[180];int n=0;
  while(*p && *p!='\n' && n<width)line[n++]=*p++;
  /* Preserve UTF-8 boundaries when wrapping. */
  while(*p && ((unsigned char)*p&0xc0)==0x80 && n<(int)sizeof(line)-1)line[n++]=*p++;
  line[n]=0;if(*p=='\n')p++;
  if(index++>=skip){vita2d_font_draw_text(f,x,y+row*22,0xffffffff,17,line);row++;}
 }
}
static void enter(void) {
 memset(&view,0,sizeof(view));view.active=-1;selected=page=scroll=file_page=0;chosen[0]=0;
 preview_frame=0;preview=vita2d_create_empty_texture(160,120);
 init_error=preview?vt_scanner_enter():-1;
}
static void input(int x,int y) {
 if(init_error<0)return;
 if(y>=44 && y<84) {page=x/240;scroll=0;return;}
 if(y>=494) {
  if(x<240){vt_scanner_request(SCAN_SAVE,0,0,NULL);return;}
  if(x<480){vt_scanner_request(SCAN_COMPLETE,0,0,NULL);page=1;return;}
  if(x<720){if(scroll>=8)scroll-=8;else scroll=0;if(file_page)file_page--;
   else if(page==3 && view.file_offset>=VT_SCAN_FILES)vt_scanner_request(SCAN_BROWSE,0,view.file_offset-VT_SCAN_FILES,view.path);}
  else {if(scroll<6000)scroll+=8;if((file_page+1)*7<(int)view.file_count)file_page++;
   else if(page==3 && view.files_more){file_page=0;vt_scanner_request(SCAN_BROWSE,0,view.file_offset+VT_SCAN_FILES,view.path);}}return;
 }
 if(page==0) {
  if(y>=95 && y<134){selected=(selected+(x<480?VT_SCAN_TESTS-1:1))%VT_SCAN_TESTS;return;}
  if(y>=370 && y<410) {
   if(x<320)vt_scanner_request(SCAN_TEST,selected,0,chosen);
   else if(x<640)vt_scanner_cancel();
   else vt_scanner_request(SCAN_TEST,selected,0,chosen);
  }
  if(y>=422 && y<462)vt_scanner_request(SCAN_VERDICT,selected,x<320?VT_PASS:x<640?VT_FAIL:VT_SKIP,NULL);
 } else if(page==2) {
  int nominal=view.model.nominal,threshold=view.model.threshold;
  if(y>=180 && y<220){if(x<320)nominal=nominal<=100?0:nominal-100;else if(x<640)nominal=nominal>=10000?10000:nominal+100;else nominal=0;vt_scanner_request(SCAN_SETTINGS,0,nominal,NULL);}
  if(y>=300 && y<340){threshold+=x<480?-1:1;vt_scanner_request(SCAN_SETTINGS,1,threshold,NULL);}
 } else if(page==3) {
  if(y>=95 && y<134) {const char *roots[]={"ux0:","uma0:","imc0:"};file_page=0;vt_scanner_request(SCAN_BROWSE,0,0,roots[x/320]);}
  if(y>=140 && y<179) {char path[512];snprintf(path,sizeof(path),"%s",view.path);char *last=strrchr(path,'/');if(last)*last=0;file_page=0;vt_scanner_request(SCAN_BROWSE,0,0,path);}
  if(y>=210 && y<210+7*38) {
   unsigned i=file_page*7+(y-210)/38;
   if(i<view.file_count) {
    char path[512];int n=snprintf(path,sizeof(path),"%s/%s",view.path,view.files[i].name);
    if(n<0 || n>=(int)sizeof(path))return;
    if(view.files[i].directory){file_page=0;vt_scanner_request(SCAN_BROWSE,0,0,path);}
    else {snprintf(chosen,sizeof(chosen),"%s",path);selected=10;page=0;}
   }
  }
 }
}
static void update(const VtSnapshot *s) {
 (void)s;vt_scanner_snapshot(&view);
 if(preview && view.preview && view.frame!=preview_frame) {
  vita2d_wait_rendering_done();
  unsigned stride=vita2d_texture_get_stride(preview);unsigned char *dest=vita2d_texture_get_datap(preview);
  for(unsigned y=0;y<120;y++)memcpy(dest+y*stride,view.pixels+y*160,160*4);
  preview_frame=view.frame;
 }
}
static void render(vita2d_font *f,const VtSnapshot *s,unsigned fps) {
 (void)s;(void)fps;
 vita2d_font_draw_text(f,15,28,0xffffffff,23,"Scanner");
 double health=vt_scan_health(&view.model);
 unsigned alert=RGBA8(255,130,75,255);
 if(view.model.battery && view.model.soh>=0 && view.model.soh<=100)
  vita2d_font_draw_textf(f,180,28,view.model.soh<view.model.threshold?alert:0xffffffff,20,"SOH sistema: %d%%",view.model.soh);
 else vita2d_font_draw_text(f,180,28,0xffffffff,20,"SOH: indisponivel");
 if(health>=0)vita2d_font_draw_textf(f,500,28,health<view.model.threshold?alert:0xffffffff,20,"Calculada: %.1f%%",health);
 else vita2d_font_draw_text(f,500,28,0xffffffff,20,"Calculada: indisponivel");
 const char *tabs[]={"Testes","Relatorio","Ajustes","Arquivo"};for(int i=0;i<4;i++)button(f,i*240,44,240,tabs[i]);
 if(init_error<0) {vita2d_font_draw_textf(f,20,160,RGBA8(255,100,100,255),22,"Falha ao iniciar Scanner: %d. Retorne pelo gesto.",init_error);return;}
 if(page==0) {
  button(f,0,95,480,"< Etapa anterior");button(f,480,95,480,"Proxima etapa >");
  vita2d_font_draw_textf(f,20,162,0xffffffff,24,"%d/%d — %s",selected+1,VT_SCAN_TESTS,vt_scan_names[selected]);
  lines(f,instructions[selected],20,194,100,2,0);
  if(selected==10)lines(f,chosen[0]?chosen:"Nenhum arquivo selecionado.",20,240,100,2,0);
  else if(view.preview && selected==view.active && preview)vita2d_draw_texture_scale(preview,630,222,1.2f,1.2f);
  lines(f,view.model.tests[selected].detail,20,277,view.preview?64:100,3,0);
  const char *verdicts[]={"NAO CONCLUIDO","PASSOU","FALHOU","PULADO"};
  vita2d_font_draw_textf(f,20,356,0xffffffff,17,"Veredito: %s | erro tecnico: %d",verdicts[view.model.tests[selected].verdict],view.model.tests[selected].error);
  button(f,0,370,320,"Iniciar");button(f,320,370,320,"Parar");button(f,640,370,320,"Repetir");
  button(f,0,422,320,"PASSOU");button(f,320,422,320,"FALHOU");button(f,640,422,320,"PULADO");
 } else if(page==1)lines(f,view.report[0]?view.report:"Coleta em andamento...",14,112,105,16,scroll);
 else if(page==2) {
  vita2d_font_draw_textf(f,20,148,0xffffffff,24,"Capacidade nominal informada: %d mAh (0 = indisponivel)",view.model.nominal);
  button(f,0,180,320,"-100 mAh");button(f,320,180,320,"+100 mAh");button(f,640,180,320,"Sem nominal");
  vita2d_font_draw_textf(f,20,268,0xffffffff,24,"Saude abaixo de %d%%: alerta de triagem",view.model.threshold);
  button(f,0,300,480,"-1%");button(f,480,300,480,"+1%");
  lines(f,"Use a capacidade nominal da bateria instalada. Sem nominal valida, o calculo fica indisponivel. SOH do sistema permanece separado. Ajustes sao salvos localmente.",20,380,100,4,0);
 } else {
  button(f,0,95,320,"ux0:");button(f,320,95,320,"uma0:");button(f,640,95,320,"imc0:");button(f,0,140,960,"Subir pasta");
  lines(f,view.path,12,202,104,1,0);
  for(unsigned row=0;row<7;row++) {unsigned i=file_page*7+row;if(i>=view.file_count)break;char b[100];snprintf(b,sizeof(b),"%s %.85s",view.files[i].directory?"[Pasta]":"[Arquivo]",view.files[i].name);button(f,0,210+row*38,960,b);}
 }
 lines(f,view.status,12,485,108,1,0);
 button(f,0,494,240,view.save_failed?"Repetir gravacao":"Salvar");button(f,240,494,240,"Concluir");button(f,480,494,240,"Pagina anterior");button(f,720,494,240,"Proxima pagina");
}
static void event(void) {vt_scanner_cancel();}
static bool leave(const VtSnapshot *s) {
 (void)s;
 if(init_error>=0 && !vt_scanner_leave())return false;
 vita2d_wait_rendering_done();if(preview)vita2d_free_texture(preview);preview=NULL;memset(&view,0,sizeof(view));return true;
}
const VtModule vt_module_scanner={.id="scanner",.name="Scanner",.enter=enter,.input=input,.update=update,.render=render,.system_event=event,.leave=leave};
