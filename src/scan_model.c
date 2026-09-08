#include "scan_model.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
const char *const vt_scan_names[]={"Acelerometro","Giroscopio","Magnetometro","Audio esquerdo","Audio direito","Microfone","Camera frontal","Camera traseira","Wi-Fi","Bluetooth","Slots / leitura"};
void vt_scan_init(VtScan *s,int64_t at) {memset(s,0,sizeof(*s));s->started=at;s->threshold=80;s->soh=s->full=s->remaining=-1;}
void vt_scan_add(VtScan *s,const char *name,const char *value,const char *unit,const char *source,VtAvailability avail,bool alert,int64_t at) {
 if(s->count>=VT_SCAN_ROWS) return;
 VtScanValue *v=&s->values[s->count++];
 if(s->count==VT_SCAN_ROWS) {
  name="Limite de inventario";value="Inventario truncado; cobertura incompleta";unit="";source="Limite de memoria";avail=VT_UNAVAILABLE;alert=true;
 }
 snprintf(v->name,sizeof(v->name),"%s",name);snprintf(v->value,sizeof(v->value),"%s",value);
 snprintf(v->unit,sizeof(v->unit),"%s",unit);snprintf(v->source,sizeof(v->source),"%s",source);
 v->availability=avail;v->alert=alert;v->at=at;
}
double vt_scan_health(const VtScan *s) {
 if(!s->battery || s->nominal<100 || s->nominal>10000 || s->full<=0 || s->full>10000 || s->remaining<0 || s->remaining>s->full) return -1;
 double v=100.0*s->full/s->nominal;return v<=120?v:-1;
}
bool vt_scan_confirm(VtScan *s,unsigned i,VtVerdict v,int64_t at) {
 if(i>=VT_SCAN_TESTS || v<=VT_PENDING || v>VT_SKIP) return false;
 if(v!=VT_SKIP && (!s->tests[i].attempted || !s->tests[i].available)) return false;
 s->tests[i].verdict=v;s->tests[i].at=at;return true;
}
bool vt_scan_complete(VtScan *s) {
 for(unsigned i=0;i<VT_SCAN_TESTS;i++) if(s->tests[i].verdict==VT_PENDING) return false;
 s->complete=true;return true;
}
static void append(char *out,size_t cap,size_t *used,const char *fmt,...) {
 if(*used>=cap) return;
 va_list args;va_start(args,fmt);int n=vsnprintf(out+*used,cap-*used,fmt,args);va_end(args);
 if(n<0 || (size_t)n>=cap-*used) {*used=cap;return;}*used+=(size_t)n;
}
size_t vt_scan_report(const VtScan *s,char *out,size_t cap) {
 size_t n=0;double health=vt_scan_health(s);
 bool soh=s->battery && s->soh>=0 && s->soh<=100;
 append(out,cap,&n,"VitaTester 1.5.0 — Scanner\nInicio UTC epoch: %lld\nFim UTC epoch: %lld\nEstado: %s\n\nREPROVACOES E ALERTAS\n",(long long)s->started,(long long)s->ended,s->complete?"CONCLUIDO":"PARCIAL");
 if(soh && s->soh<s->threshold) append(out,cap,&n,"ALERTA: SOH scePower %d%% abaixo de %d%% (triagem).\n",s->soh,s->threshold);
 if(health>=0 && health<s->threshold) append(out,cap,&n,"ALERTA: saude calculada %.2f%% abaixo de %d%% (triagem).\n",health,s->threshold);
 if(s->battery && (s->soh>100 || s->full==0 || s->full>10000 || s->remaining>s->full || (s->nominal && health<0))) append(out,cap,&n,"ALERTA: leituras de capacidade/saude inconsistentes; calculo invalidado.\n");
 for(unsigned i=0;i<s->count;i++) if(s->values[i].alert || s->values[i].availability==VT_ERROR) append(out,cap,&n,"ALERTA: %s: %s\n",s->values[i].name,s->values[i].value);
 for(unsigned i=0;i<VT_SCAN_TESTS;i++) {
  if(s->tests[i].verdict==VT_FAIL) append(out,cap,&n,"FALHOU: %s (confirmacao humana)\n",vt_scan_names[i]);
  if(s->tests[i].error) append(out,cap,&n,"ERRO operacional: %s rc=%d; %s\n",vt_scan_names[i],s->tests[i].error,s->tests[i].detail);
 }
 append(out,cap,&n,"\nSAUDE DA BATERIA — criterio de triagem: abaixo de %d%%\n",s->threshold);
 if(soh) append(out,cap,&n,"SOH: %d %% | fonte: scePowerGetBatterySOH\n",s->soh);
 else append(out,cap,&n,"SOH: INDISPONIVEL (sem bateria ou leitura invalida)\n");
 if(health>=0) append(out,cap,&n,"Calculada: %.2f %% | 100 x completa %d mAh / nominal informada %d mAh\n",health,s->full,s->nominal);
 else append(out,cap,&n,"Calculada: INDISPONIVEL | nominal informada: %d mAh\n",s->nominal);
 append(out,cap,&n,"SOH e calculo sao medidas separadas, sem media.\n\nINVENTARIO (horarios UTC epoch)\n");
 for(unsigned i=0;i<s->count;i++) {const VtScanValue *v=&s->values[i];append(out,cap,&n,"%s: %s %s\n  fonte: %s | t=%lld | %s\n",v->name,v->value,v->unit,v->source,(long long)v->at,v->availability==VT_AVAILABLE?"DISPONIVEL":v->availability==VT_ERROR?"ERRO DE ACESSO":"INDISPONIVEL");}
 append(out,cap,&n,"\nTESTES — vereditos humanos, erros tecnicos separados\n");
 const char *names[]={"NAO CONCLUIDO","PASSOU","FALHOU","PULADO"};
 for(unsigned i=0;i<VT_SCAN_TESTS;i++) append(out,cap,&n,"%s: %s | tentativas=%u | t=%lld | rc=%d\n  %s\n",vt_scan_names[i],names[s->tests[i].verdict],s->tests[i].attempts,(long long)s->tests[i].at,s->tests[i].error,s->tests[i].detail);
 append(out,cap,&n,"\nINDISPONIVEIS / PULADOS / COBERTURA INCOMPLETA\n");
 for(unsigned i=0;i<s->count;i++) if(s->values[i].availability!=VT_AVAILABLE) append(out,cap,&n,"%s\n",s->values[i].name);
 for(unsigned i=0;i<VT_SCAN_TESTS;i++) if(s->tests[i].verdict==VT_SKIP || s->tests[i].verdict==VT_PENDING || !s->tests[i].available) append(out,cap,&n,"%s: %s\n",vt_scan_names[i],names[s->tests[i].verdict]);
 append(out,cap,&n,"Este relatorio nao certifica aprovacao integral do aparelho.\nAusencia opcional/API inacessivel nao prova defeito.\nSem audio, imagens, conteudo de dumps/arquivos ou identificadores de rede.\n");
 if(n>=cap) {if(cap)out[cap-1]=0;return 0;}return n;
}
