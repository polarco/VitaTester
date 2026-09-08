#include "scanner.h"
#include "runtime.h"
#ifdef VT_SCANNER_HOST
#include "scanner_vita.h"
#else
#include <psp2/power.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/vshbridge.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/io/dirent.h>
#include <psp2/io/devctl.h>
#include <psp2/motion.h>
#include <psp2/audioout.h>
#include <psp2/audioin.h>
#include <psp2/camera.h>
#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <psp2/sysmodule.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <malloc.h>
#define ROOT "ux0:data/VitaTester"
static VtScannerView work,shared;
static SceUID worker=-1,lock=-1;
static atomic_int job;
static atomic_bool cancel,done;
static int arg_test,arg_value;
static char arg_path[512];
static unsigned epoch,request_epoch;
static atomic_bool exiting;
static SceUID camera_mem=-1;
static void *camera_buffer;
static int camera_device=-1,audio_port=-1,mic_port=-1;
static bool motion_on,magnetometer_on;
/* Retain ownership on cleanup failure. No later test/module may reuse devices. */
static bool release_devices(void) {
 bool ok=true;
 if(magnetometer_on) {if(sceMotionMagnetometerOff()>=0)magnetometer_on=false;else ok=false;}
 if(motion_on) {if(sceMotionStopSampling()>=0)motion_on=false;else ok=false;}
 if(audio_port>=0) {if(sceAudioOutReleasePort(audio_port)>=0)audio_port=-1;else ok=false;}
 if(mic_port>=0) {if(sceAudioInReleasePort(mic_port)>=0)mic_port=-1;else ok=false;}
 if(camera_device>=0) {
  sceCameraStop(camera_device);
  if(sceCameraClose(camera_device)>=0)camera_device=-1;else ok=false;
 }
 if(camera_device<0 && camera_mem>=0) {
  memset(camera_buffer,0,VT_CAMERA_BYTES);
  if(sceKernelFreeMemBlock(camera_mem)>=0){camera_mem=-1;camera_buffer=NULL;}else ok=false;
 }
 return ok;
}
static void publish(void) {
 sceKernelLockMutex(lock,1,NULL);shared=work;sceKernelUnlockMutex(lock,1);
}
static bool stopped(void) {return atomic_load(&cancel) || epoch!=vt_runtime_epoch() || !vt_runtime_focused();}
static void add(const char *name,const char *value,const char *unit,const char *source,VtAvailability avail,bool alert) {
 vt_scan_add(&work.model,name,value,unit,source,avail,alert,time(NULL));
}
static void number(const char *name,int v,const char *unit,const char *source) {
 char buf[64];snprintf(buf,sizeof(buf),"%d",v);add(name,buf,unit,source,v<0?VT_ERROR:VT_AVAILABLE,false);
}
static void clean(char *s) {for(;*s;s++) if((unsigned char)*s<32 || *s==127) *s=' ';}
static void settings_load(void) {
 FILE *f=fopen(ROOT "/scanner.cfg","r");if(!f)return;
 int nominal=0,threshold=80;
 if(fscanf(f,"%d %d",&nominal,&threshold)==2 && (nominal==0 || (nominal>=100 && nominal<=10000)) && threshold>=1 && threshold<=100) {work.model.nominal=nominal;work.model.threshold=threshold;}
 fclose(f);
}
/* Complete writes and sync before rename. The previous file survives any failed stage. */
static int replace_file(const char *path,const void *data,size_t size) {
 char temp[300];if(snprintf(temp,sizeof(temp),"%s.tmp",path)>=(int)sizeof(temp)) return -1;
 SceUID fd=sceIoOpen(temp,SCE_O_WRONLY|SCE_O_CREAT|SCE_O_TRUNC,0600);if(fd<0)return fd;
 const char *bytes=data;size_t at=0;int rc=0;
 while(at<size) {int n=sceIoWrite(fd,bytes+at,size-at>4096?4096:size-at);if(n<=0){rc=n<0?n:-1;break;}at+=(unsigned)n;}
 int closed=sceIoClose(fd);if(!rc && closed<0)rc=closed;
 if(!rc) rc=sceIoSync("ux0:",0);
 if(!rc) rc=sceIoRename(temp,path);
 if(!rc) rc=sceIoSync("ux0:",0);
 return rc;
}
static bool save(void) {
 work.model.ended=time(NULL);
 size_t n=vt_scan_report(&work.model,work.report,sizeof(work.report));
 int rc=n?0:-1;
 if(!work.report_path[0]) {
  char date[32];time_t at=work.model.started;struct tm tm;gmtime_r(&at,&tm);strftime(date,sizeof(date),"%Y%m%d_%H%M%S",&tm);
  for(unsigned i=0;i<10000;i++) {
   char path[256];
   if(i)snprintf(path,sizeof(path),ROOT "/scan_%s_%u.txt",date,i);
   else snprintf(path,sizeof(path),ROOT "/scan_%s.txt",date);
   SceUID fd=sceIoOpen(path,SCE_O_WRONLY|SCE_O_CREAT|SCE_O_EXCL,0600);
   if(fd>=0) {sceIoClose(fd);snprintf(work.report_path,sizeof(work.report_path),"%s",path);break;}
   SceIoStat st;if(sceIoGetstat(path,&st)<0) {rc=fd;break;}
  }
  if(!work.report_path[0])rc=-1;
 }
 if(!rc)rc=replace_file(work.report_path,work.report,n);
 work.save_failed=rc<0;
 if(rc<0)snprintf(work.status,sizeof(work.status),"Falha ao salvar (%d). Resultado preservado; toque Salvar para repetir.",rc);
 publish();return !work.save_failed;
}
static void dump_metadata(const char *dir) {
 int fd=sceIoDopen(dir);if(fd<0) {number(dir,fd,"rc","sceIoDopen: metadados dumps");return;}
 SceIoDirent ent;int rc;unsigned found=0;
 while(!atomic_load(&cancel)) {
  memset(&ent,0,sizeof(ent));rc=sceIoDread(fd,&ent);if(rc<=0)break;
  if(strncmp(ent.d_name,"psp2core",8) || !SCE_S_ISREG(ent.d_stat.st_mode))continue;
  char value[256];clean(ent.d_name);
  snprintf(value,sizeof(value),"%.140s | %llu bytes | %04u-%02u-%02u %02u:%02u:%02u",ent.d_name,(unsigned long long)ent.d_stat.st_size,ent.d_stat.st_mtime.year,ent.d_stat.st_mtime.month,ent.d_stat.st_mtime.day,ent.d_stat.st_mtime.hour,ent.d_stat.st_mtime.minute,ent.d_stat.st_mtime.second);
  add("Crash dump (metadados; nao indica falha atual)",value,"",dir,VT_AVAILABLE,false);
  if(++found==32){add("Dumps","Listagem limitada a 32 por diretorio","",dir,VT_UNAVAILABLE,false);break;}
 }
 sceIoDclose(fd);
 if(rc<0)number(dir,rc,"rc","sceIoDread");
 if(!found && rc==0)add("Dumps","Nenhum psp2core regular listado","",dir,VT_AVAILABLE,false);
}
static void configuration(const char *path) {
 int fd=sceIoOpen(path,SCE_O_RDONLY,0);if(fd<0){number(path,fd,"rc","Arquivo de configuracao: acesso");return;}
 char text[8193];int n=sceIoRead(fd,text,sizeof(text)-1);sceIoClose(fd);
 if(n<0){number(path,n,"rc","sceIoRead configuracao");return;}text[n]=0;
 add("Configuracao acessivel",path,"","Arquivo; nao comprova carregamento",VT_AVAILABLE,false);
 char *saveptr=NULL;
 for(char *line=strtok_r(text,"\r\n",&saveptr);line;line=strtok_r(NULL,"\r\n",&saveptr)) {
  while(*line==' ' || *line=='\t')line++;
  if(*line=='#' || *line=='*')continue;
  char *comment=strchr(line,'#');if(comment)*comment=0;
  if(strstr(line,".skprx") || strstr(line,".suprx")) {clean(line);add("Plugin CONFIGURADO",line,"",path,VT_AVAILABLE,false);}
  if(strstr(line,"GCD=") || strstr(line,"yamt") || strstr(line,"storagemgr") || strstr(line,"gamesd"))add("Evidencia SD2Vita/redirecionamento","Configuracao encontrada; dispositivo fisico inconclusivo","",path,VT_AVAILABLE,false);
 }
 if(n==8192)add("Configuracao","Leitura limitada a 8192 bytes","",path,VT_UNAVAILABLE,false);
}
static void collect(void) {
 settings_load();
 int tv=sceKernelIsPSVitaTV();work.model.battery=tv==0;
 number("Tipo (0 Vita / 1 Vita TV)",tv,"tipo","sceKernelIsPSVitaTV");
 work.model.soh=scePowerGetBatterySOH();work.model.full=scePowerGetBatteryFullCapacity();work.model.remaining=scePowerGetBatteryRemainCapacity();
 number("SOH",work.model.soh,"%","scePowerGetBatterySOH");
 number("Capacidade completa",work.model.full,"mAh","scePowerGetBatteryFullCapacity");
 number("Capacidade restante",work.model.remaining,"mAh","scePowerGetBatteryRemainCapacity");
 number("Temperatura",scePowerGetBatteryTemp(),"0.01 C","scePowerGetBatteryTemp");
 number("Tensao",scePowerGetBatteryVolt(),"mV","scePowerGetBatteryVolt");
 number("Carga",scePowerGetBatteryLifePercent(),"%","scePowerGetBatteryLifePercent");
 if(tv!=0)for(unsigned i=1;i<work.model.count;i++)work.model.values[i].availability=VT_UNAVAILABLE;
 SceKernelSystemSwVersion fw={.size=sizeof(fw)};int rc=sceKernelGetSystemSwVersion(&fw);
 if(rc<0)number("Firmware reportado",rc,"rc","sceKernelGetSystemSwVersion");
 else {fw.versionString[sizeof(fw.versionString)-1]=0;add("Firmware reportado (pode ter spoof)",fw.versionString,"","sceKernelGetSystemSwVersion",VT_AVAILABLE,false);}
 memset(&fw,0,sizeof(fw));fw.size=sizeof(fw);rc=_vshSblGetSystemSwVersion(&fw);
 if(rc<0)number("Firmware real: acesso indisponivel",rc,"rc","_vshSblGetSystemSwVersion (SDK)");
 else {fw.versionString[sizeof(fw.versionString)-1]=0;add("Firmware real consultado",fw.versionString,"","_vshSblGetSystemSwVersion (SDK)",VT_AVAILABLE,false);}
 add("Modelo PCH exato / regiao","Sem identificacao confiavel","","API userland",VT_UNAVAILABLE,false);
 SceKernelFreeMemorySizeInfo mem={.size=sizeof(mem)};rc=sceKernelGetFreeMemorySize(&mem);
 if(rc<0)number("Memoria livre processo",rc,"rc","sceKernelGetFreeMemorySize");
 else {number("Memoria user livre",mem.size_user,"bytes","sceKernelGetFreeMemorySize");number("CDRAM livre",mem.size_cdram,"bytes","sceKernelGetFreeMemorySize");number("PHYCONT livre",mem.size_phycont,"bytes","sceKernelGetFreeMemorySize");}
 const char *mounts[]={"ux0:","uma0:","imc0:"};
 for(unsigned i=0;i<3;i++) {
  SceIoDevInfo info={0};rc=sceIoDevctl(mounts[i],0x3001,NULL,0,&info,sizeof(info));
  if(rc<0)number(mounts[i],rc,"rc","sceIoDevctl 0x3001: acesso, nao ausencia");
  else {char b[160];snprintf(b,sizeof(b),"total=%llu livre=%llu; fisico inconclusivo",(unsigned long long)info.max_size,(unsigned long long)info.free_size);add(mounts[i],b,"bytes","sceIoDevctl 0x3001",VT_AVAILABLE,info.free_size==0 || info.free_size>info.max_size);}
 }
 configuration("ur0:tai/config.txt");configuration("ux0:tai/config.txt");configuration("ur0:tai/storage_config.txt");
 SceIoStat st;rc=sceIoGetstat("ur0:tai/boot_config.txt",&st);
 add("Enso: evidencia de arquivo",rc>=0?"boot_config.txt acessivel; nao prova boot persistente ativo":"Arquivo nao consultavel; estado de boot inconclusivo","","sceIoGetstat",rc>=0?VT_AVAILABLE:VT_UNAVAILABLE,false);
 unsigned char probe[8]={0};rc=_vshKernelSearchModuleByName("taihen",probe);
 number("taiHEN consultavel (UID ou erro)",rc,"UID/rc","_vshKernelSearchModuleByName: SDK");
 add("Inventario kernel","Nao acessivel por este inventario userland","","SDK fixado",VT_UNAVAILABLE,false);
 SceUID ids[64];SceSize count=64;rc=sceKernelGetModuleList(0xff,ids,&count);
 if(rc<0)number("Modulos do processo",rc,"rc","sceKernelGetModuleList");
 else {
  if(count>=64)add("Modulos","Listagem limitada a 64","","sceKernelGetModuleList",VT_UNAVAILABLE,false);
  for(unsigned i=0;i<count && i<64;i++) {SceKernelModuleInfo info={.size=sizeof(info)};rc=sceKernelGetModuleInfo(ids[i],&info);if(rc>=0){info.module_name[27]=0;clean(info.module_name);add("Modulo VISIVEL no processo",info.module_name,"","sceKernelGetModuleInfo",VT_AVAILABLE,false);}else number("Informacao de modulo",rc,"rc","sceKernelGetModuleInfo");}
 }
 dump_metadata("ux0:data");dump_metadata("ux0:");dump_metadata("ud0:PSP2CORE");
 add("Wi-Fi: varredura AP / Bluetooth","INDISPONIVEL: sem operacao userland documentada nesta imagem","","SDK fixado",VT_UNAVAILABLE,false);
 snprintf(work.status,sizeof(work.status),"Coleta passiva terminada. Selecione um teste e toque Iniciar.");
}
static void browse(const char *path,unsigned offset) {
 work.file_offset=offset;unsigned eligible=0;
 work.file_count=0;work.files_more=false;snprintf(work.path,sizeof(work.path),"%s",path);
 int fd=sceIoDopen(path);if(fd<0){snprintf(work.status,sizeof(work.status),"Diretorio inacessivel: %d",fd);return;}
 SceIoDirent e;int rc=0;
 while(!atomic_load(&cancel)) {
  memset(&e,0,sizeof(e));rc=sceIoDread(fd,&e);if(rc<=0)break;
  if(!strcmp(e.d_name,".") || !strcmp(e.d_name,"..") || !strncmp(e.d_name,"psp2core",8))continue;
  if(!SCE_S_ISDIR(e.d_stat.st_mode) && !SCE_S_ISREG(e.d_stat.st_mode))continue;
  if(eligible++<offset)continue;
  if(work.file_count==VT_SCAN_FILES){work.files_more=true;break;}
  VtScanFile *f=&work.files[work.file_count++];snprintf(f->name,sizeof(f->name),"%s",e.d_name);f->directory=SCE_S_ISDIR(e.d_stat.st_mode);
 }
 sceIoDclose(fd);
 snprintf(work.status,sizeof(work.status),"%u entradas%s; leitura limitada a 1 MiB, sem escrita.",work.file_count,work.files_more?" (ha mais; use Proxima pagina)":"");
 if(rc<0)snprintf(work.status,sizeof(work.status),"Erro de listagem: %d",rc);
}
static int sensor(int test,VtScanTest *t) {
 int rc=sceMotionStartSampling();if(rc<0)return rc;motion_on=true;
 
 if(test==2){rc=sceMotionMagnetometerOn();magnetometer_on=rc>=0;}
 uint64_t start=sceKernelGetProcessTimeWide();
 while(rc>=0 && !stopped() && sceKernelGetProcessTimeWide()-start<30000000) {
  SceMotionState m;rc=sceMotionGetState(&m);if(rc<0)break;
  t->available=true;
  if(test<2) {SceFVector3 v=test==0?m.acceleration:m.angularVelocity;snprintf(t->detail,sizeof(t->detail),"sceMotionGetState: x=%.4f y=%.4f z=%.4f | device=%u host=%llu us",v.x,v.y,v.z,m.timestamp,(unsigned long long)m.hostTimestamp);}
  else {float matrix[16];memcpy(matrix,&m.nedMatrix,sizeof(matrix));snprintf(t->detail,sizeof(t->detail),"NED [%.2f %.2f %.2f; %.2f %.2f %.2f; %.2f %.2f %.2f] estabilidade=%u | device=%u host=%llu",matrix[0],matrix[1],matrix[2],matrix[4],matrix[5],matrix[6],matrix[8],matrix[9],matrix[10],m.magFieldStability,m.timestamp,(unsigned long long)m.hostTimestamp);}
  snprintf(work.status,sizeof(work.status),"%.255s",t->detail);publish();sceKernelDelayThread(50000);
 }
 if(!release_devices() && rc>=0)rc=-1;
 return rc;
}
static int tone(int test,VtScanTest *t) {
 int port=sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_MAIN,256,48000,SCE_AUDIO_OUT_MODE_STEREO);if(port<0)return port;audio_port=port;
 int16_t samples[512];int rc=0;unsigned total=48000*3;int16_t last=0;
 for(unsigned at=0;at<total && !stopped();at+=256) {
  memset(samples,0,sizeof(samples));
  for(unsigned i=0;i<256 && at+i<total;i++) {
   unsigned k=at+i;double ramp=fmin(1.0,fmin(k/2400.0,(total-1-k)/2400.0));
   samples[2*i+(test==4)]=(int16_t)(3276*ramp*sin(2*3.141592653589793*440*k/48000));
  }
  last=samples[510+(test==4)];
  rc=sceAudioOutOutput(port,samples);if(rc<0)break;t->available=true;
 }
 /* Short buffers bound stop latency; no global-volume API is used. */
 memset(samples,0,sizeof(samples));
 for(unsigned i=0;i<256;i++)samples[2*i+(test==4)]=(int16_t)(last*(255-(int)i)/255);
 int e=sceAudioOutOutput(port,samples);if(rc>=0 && e<0)rc=e;
 if(!release_devices() && rc>=0)rc=-1;
 snprintf(t->detail,sizeof(t->detail),"440 Hz, canal %s, 3 s, pico -20 dBFS, rampas 50 ms; %s",test==3?"esquerdo":"direito",stopped()?"interrompido":"terminado");return rc;
}
static int microphone(VtScanTest *t) {
 int port=sceAudioInOpenPort(SCE_AUDIO_IN_PORT_TYPE_VOICE,256,16000,SCE_AUDIO_IN_PARAM_FORMAT_S16_MONO);if(port<0)return port;mic_port=port;
 int16_t samples[256];int rc=0;uint64_t start=sceKernelGetProcessTimeWide();
 while(!stopped() && sceKernelGetProcessTimeWide()-start<10000000) {
  rc=sceAudioInInput(port,samples);if(rc<0)break;double sum=0,peak=0;
  for(unsigned i=0;i<256;i++){double v=samples[i]/32768.0;sum+=v*v;if(fabs(v)>peak)peak=fabs(v);}
  t->available=true;snprintf(t->detail,sizeof(t->detail),"sceAudioInInput: RMS %.1f dBFS | pico %.1f dBFS | t=%llu us; sem gravacao",20*log10(fmax(sqrt(sum/256),1e-6)),20*log10(fmax(peak,1e-6)),(unsigned long long)sceKernelGetProcessTimeWide());
  snprintf(work.status,sizeof(work.status),"%.255s",t->detail);publish();
 }
 memset(samples,0,sizeof(samples));if(!release_devices() && rc>=0)rc=-1;return rc;
}
static int camera(int test,VtScanTest *t) {
 int dev=test==6?SCE_CAMERA_DEVICE_FRONT:SCE_CAMERA_DEVICE_BACK;
 camera_mem=sceKernelAllocMemBlock("vt_camera",SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW,256*1024,NULL);
 if(camera_mem<0)return camera_mem;
 int base_rc=sceKernelGetMemBlockBase(camera_mem,&camera_buffer);
 if(base_rc<0){sceKernelFreeMemBlock(camera_mem);camera_mem=-1;return base_rc;}
 void *buffer=camera_buffer;memset(buffer,0,VT_CAMERA_BYTES);
 SceCameraInfo info={.size=sizeof(info),.priority=SCE_CAMERA_PRIORITY_SHARE,.format=SCE_CAMERA_FORMAT_ABGR,.resolution=SCE_CAMERA_RESOLUTION_160_120,.framerate=SCE_CAMERA_FRAMERATE_30_FPS,.sizeIBase=VT_CAMERA_BYTES,.pIBase=buffer};
 int rc=sceCameraOpen(dev,&info);
 if(rc>=0){camera_device=dev;rc=sceCameraStart(dev);}
 uint64_t at=sceKernelGetProcessTimeWide();
 while(rc>=0 && !stopped() && sceKernelGetProcessTimeWide()-at<30000000) {
  SceCameraRead read={.size=sizeof(read),.mode=1,.sizeIBase=VT_CAMERA_BYTES,.pIBase=buffer};
  rc=sceCameraRead(dev,&read);
  if(rc==(int)SCE_CAMERA_ERROR_ALREADY_READ || rc==(int)SCE_CAMERA_ERROR_TIMEOUT) {rc=0;sceKernelDelayThread(10000);continue;}
  if(rc<0)break;
  memcpy(work.pixels,buffer,VT_CAMERA_BYTES);work.preview=true;work.frame++;t->available=true;
  snprintf(t->detail,sizeof(t->detail),"sceCameraRead: frame=%llu timestamp=%llu; preview temporario",(unsigned long long)read.frame,(unsigned long long)read.timestamp);
  publish();sceKernelDelayThread(33333);
 }
 work.preview=false;memset(work.pixels,0,sizeof(work.pixels));
 if(!release_devices() && rc>=0)rc=-1;
 return rc;
}
static int wifi(VtScanTest *t) {
 int rc=sceSysmoduleLoadModule(SCE_SYSMODULE_NET);bool loaded=rc>=0;
 void *memory=malloc(128*1024);if(!memory){if(loaded)sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);return -1;}
 SceNetInitParam param={.memory=memory,.size=128*1024,.flags=0};
 rc=sceNetInit(&param);bool ownnet=rc>=0;
 rc=sceNetCtlInit();bool ownctl=rc>=0;
 int state=-1;rc=sceNetCtlInetGetState(&state);
 if(rc>=0) {
  SceNetCtlInfo signal={0},channel={0};int sr=sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_RSSI_PERCENTAGE,&signal),cr=sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_CHANNEL,&channel);
  t->available=true;snprintf(t->detail,sizeof(t->detail),"SceNetCtl: estado=%d (0 desligado, 3 conectado); sinal=%u%% rc=%d; canal=%u rc=%d. AP scan INDISPONIVEL.",state,signal.rssi_percentage,sr,channel.channel,cr);
 }
 if(ownctl)sceNetCtlTerm();
 if(ownnet)sceNetTerm();
 free(memory);
 if(loaded)sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
 return rc;
}
static int read_file(const char *path,VtScanTest *t) {
 int sony=vshMemoryCardGetCardInsertState(),removable=vshRemovableMemoryGetCardInsertState();
 if(strncmp(path,"ux0:",4) && strncmp(path,"uma0:",5) && strncmp(path,"imc0:",5))return -1;
 if(strstr(path,"..") || strstr(path,"psp2core"))return -1;
 SceIoStat st;int rc=sceIoGetstat(path,&st);if(rc<0)return rc;if(!SCE_S_ISREG(st.st_mode))return -1;
 int fd=sceIoOpen(path,SCE_O_RDONLY,0);if(fd<0)return fd;
 char buffer[4096];unsigned bytes=0;
 while(bytes<1024*1024 && !stopped()) {rc=sceIoRead(fd,buffer,sizeof(buffer));if(rc<=0)break;bytes+=(unsigned)rc;}
 int e=sceIoClose(fd);memset(buffer,0,sizeof(buffer));
 t->available=bytes>0;snprintf(t->detail,sizeof(t->detail),"Leitura montagem: %u bytes; %s; insercao Sony=%d removivel=%d (SDK Vsh). Mapeamento fisico inconclusivo.",bytes,stopped()?"cancelada":"terminada",sony,removable);
 return rc<0?rc:e;
}
static void run_test(int i,const char *path) {
 if(i<0 || i>=VT_SCAN_TESTS)return;
 VtScanTest *t=&work.model.tests[i];t->attempted=true;t->available=false;t->error=0;t->verdict=VT_PENDING;t->attempts++;t->at=time(NULL);work.model.complete=false;
 snprintf(t->detail,sizeof(t->detail),"Iniciado; aguarde resultado e confirme o veredito.");
 work.active=i;publish();int rc=0;
 if(stopped())snprintf(t->detail,sizeof(t->detail),"Interrompido antes de abrir dispositivo. Inicie novamente.");
 else if(i<3)rc=sensor(i,t);
 else if(i<5)rc=tone(i,t);
 else if(i==5)rc=microphone(t);
 else if(i<8)rc=camera(i,t);
 else if(i==8)rc=wifi(t);
 else if(i==9)snprintf(t->detail,sizeof(t->detail),"INDISPONIVEL: SDK sem API Bluetooth userland documentada; confirme PULADO.");
 else rc=read_file(path,t);
 if(rc<0){char failure[128];snprintf(failure,sizeof(failure),"%s: rc=%d tentativa=%u",vt_scan_names[i],rc,t->attempts);add("Erro operacional de teste",failure,"rc","Worker Scanner",VT_ERROR,true);t->error=rc;snprintf(work.status,sizeof(work.status),"Erro tecnico %d. Sem reprovacao automatica; confirme ou repita.",rc);}
 else snprintf(work.status,sizeof(work.status),"%s",stopped()?"Teste interrompido. Novo inicio exige toque explicito.":"Teste terminado. Confirme PASSOU / FALHOU / PULADO.");
 if(stopped()) {
  size_t len=strlen(t->detail);snprintf(t->detail+len,sizeof(t->detail)-len," | interrompido");
 }
 work.active=-1;work.preview=false;t->at=time(NULL);
}
static int loop(SceSize size,void *arg) {
 (void)size;(void)arg;
 while(!atomic_load(&done)) {
  int next=atomic_load(&job);if(!next){sceKernelDelayThread(10000);continue;}
  int test=arg_test,value=arg_value;char path[512];snprintf(path,sizeof(path),"%s",arg_path);
  epoch=request_epoch;
  work.busy=true;publish();
  if(!release_devices()) {
   snprintf(work.status,sizeof(work.status),"Falha ao liberar dispositivo: transicao bloqueada. Tentando novamente.");
   work.busy=false;publish();atomic_store(&job,0);sceKernelDelayThread(100000);continue;
  }
  if(next==SCAN_COLLECT){sceIoMkdir("ux0:data",0777);sceIoMkdir(ROOT,0777);collect();save();}
  else if(next==SCAN_TEST){run_test(test,path);save();}
  else if(next==SCAN_BROWSE)browse(path,value<0?0:(unsigned)value);
  else if(next==SCAN_VERDICT){vt_scan_confirm(&work.model,test,(VtVerdict)value,time(NULL));save();}
  else if(next==SCAN_SETTINGS) {
   if(test==0 && (value==0 || (value>=100 && value<=10000)))work.model.nominal=value;
   if(test==1 && value>=1 && value<=100)work.model.threshold=value;
   char data[48];int n=snprintf(data,sizeof(data),"%d %d\n",work.model.nominal,work.model.threshold);
   int rc=replace_file(ROOT "/scanner.cfg",data,n);if(rc<0)add("Ajustes: falha de persistencia","Valores mantidos apenas nesta sessao","","scanner.cfg",VT_ERROR,true);
   save();
  } else if(next==SCAN_COMPLETE) {
   if(vt_scan_complete(&work.model))snprintf(work.status,sizeof(work.status),"Etapas confirmadas; cobertura indisponivel continua explicita.");
   else snprintf(work.status,sizeof(work.status),"Conclusao pendente: confirme todas as etapas, inclusive PULADO.");
   save();
  } else if(next==SCAN_SAVE) {if(save() && atomic_load(&exiting)){work.finished=true;atomic_store(&done,true);}}
  else if(next==SCAN_EXIT) {if(save()) {work.finished=true;atomic_store(&done,true);}}
  work.busy=false;publish();atomic_store(&job,0);
 }
 return 0;
}
int vt_scanner_enter(void) {
 memset(&work,0,sizeof(work));vt_scan_init(&work.model,time(NULL));work.active=-1;work.busy=true;
 lock=sceKernelCreateMutex("vt_scan_view",0,0,NULL);if(lock<0)return lock;
 request_epoch=vt_runtime_epoch();atomic_store(&done,false);atomic_store(&cancel,false);atomic_store(&job,SCAN_COLLECT);exiting=false;
 snprintf(work.status,sizeof(work.status),"Coletando inventario passivo...");publish();
 worker=sceKernelCreateThread("vt_scanner",loop,110,128*1024,0,0,NULL);
 if(worker<0){sceKernelDeleteMutex(lock);lock=-1;return worker;}
 int rc=sceKernelStartThread(worker,0,NULL);
 if(rc<0){sceKernelDeleteThread(worker);worker=-1;sceKernelDeleteMutex(lock);lock=-1;}
 return rc;
}
void vt_scanner_snapshot(VtScannerView *v) {if(lock>=0 && sceKernelTryLockMutex(lock,1)==0){*v=shared;sceKernelUnlockMutex(lock,1);}}
bool vt_scanner_request(int next,int test,int value,const char *path) {
 if(worker<0 || atomic_load(&job) || atomic_load(&done))return false;
 if(atomic_load(&exiting) && next!=SCAN_SAVE)return false;
 arg_test=test;arg_value=value;snprintf(arg_path,sizeof(arg_path),"%s",path?path:"");
 request_epoch=vt_runtime_epoch();atomic_store(&cancel,false);atomic_store(&job,next);return true;
}
void vt_scanner_cancel(void) {atomic_store(&cancel,true);}
bool vt_scanner_leave(void) {
 vt_scanner_cancel();
 if(atomic_load(&done)) {
  SceUInt timeout=0;if(sceKernelWaitThreadEnd(worker,NULL,&timeout)<0)return false;
  sceKernelDeleteThread(worker);worker=-1;sceKernelDeleteMutex(lock);lock=-1;return true;
 }
 if(!exiting && !atomic_load(&job)){exiting=vt_scanner_request(SCAN_EXIT,0,0,NULL);}

 return false;
}
