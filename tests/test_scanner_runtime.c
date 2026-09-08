#define VT_SCANNER_HOST
#include "../src/scanner.c"
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
static unsigned fake_epoch;
static uint64_t fake_time=1000000;
static int io_write_limit=17,sync_fail,sync_count,write_fail,rename_fail;
static int device_error,close_error,allocation_error,reads,read_cancel,dir_mode,dir_pos;
static bool focus=true;
static void *allocation;
unsigned vt_runtime_epoch(void){return fake_epoch;}
bool vt_runtime_focused(void){return focus;}
int sceKernelLockMutex(int a,int b,void *c){(void)a;(void)b;(void)c;return 0;}
int sceKernelTryLockMutex(int a,int b){(void)a;(void)b;return 0;}
int sceKernelUnlockMutex(int a,int b){(void)a;(void)b;return 0;}
int sceKernelCreateMutex(const char *a,unsigned b,int c,void *d){(void)a;(void)b;(void)c;(void)d;return 1;}
int sceKernelDeleteMutex(int n){(void)n;return 0;}
int sceKernelDeleteThread(int n){(void)n;return 0;}
int sceKernelDelayThread(unsigned n){fake_time+=n;return 0;}
uint64_t sceKernelGetProcessTimeWide(void){fake_time+=10000;return fake_time;}
int sceKernelCreateThread(const char *a,int (*f)(SceSize,void *),int b,unsigned c,unsigned d,int e,void *g){(void)a;(void)f;(void)b;(void)c;(void)d;(void)e;(void)g;return 1;}
int sceKernelStartThread(int a,unsigned b,void *c){(void)a;(void)b;(void)c;return 0;}
int sceKernelWaitThreadEnd(int a,void *b,unsigned *c){(void)a;(void)b;(void)c;return 0;}
int sceKernelAllocMemBlock(const char *n,int t,unsigned size,void *opt){(void)n;(void)t;(void)opt;if(allocation_error)return -12;allocation=malloc(size);return allocation?1:-12;}
int sceKernelGetMemBlockBase(int i,void **out){(void)i;*out=allocation;return 0;}
int sceKernelFreeMemBlock(int i){(void)i;free(allocation);allocation=NULL;return 0;}
int sceKernelIsPSVitaTV(void){return 0;}
int scePowerGetBatterySOH(void){return 75;}
int scePowerGetBatteryFullCapacity(void){return 1700;}
int scePowerGetBatteryRemainCapacity(void){return 600;}
int scePowerGetBatteryVolt(void){return 3800;}
int scePowerGetBatteryTemp(void){return 3200;}
int scePowerGetBatteryLifePercent(void){return 35;}
int sceKernelGetSystemSwVersion(SceKernelSystemSwVersion *v){strcpy(v->versionString,"3.74");return 0;}
int _vshSblGetSystemSwVersion(SceKernelSystemSwVersion *v){(void)v;return -13;}
int sceKernelGetFreeMemorySize(SceKernelFreeMemorySizeInfo *m){m->size_user=10000;m->size_cdram=5000;m->size_phycont=1000;return 0;}
int _vshKernelSearchModuleByName(const char *n,const void *b){(void)n;(void)b;return -13;}
int sceKernelGetModuleList(unsigned t,SceUID *ids,SceSize *n){(void)t;ids[0]=2;*n=1;return 0;}
int sceKernelGetModuleInfo(int id,SceKernelModuleInfo *v){(void)id;strcpy(v->module_name,"visible_only");return 0;}
int sceIoOpen(const char *p,int flags,int mode){
 int native=(flags&SCE_O_WRONLY)?O_WRONLY:O_RDONLY;
 if(flags&SCE_O_CREAT)native|=O_CREAT;
 if(flags&SCE_O_TRUNC)native|=O_TRUNC;
 if(flags&SCE_O_EXCL)native|=O_EXCL;
 int r=open(p,native,mode);return r<0?-errno:r;
}
int sceIoWrite(int fd,const void *p,unsigned n){if(write_fail)return 0;if(n>(unsigned)io_write_limit)n=io_write_limit;int r=write(fd,p,n);return r<0?-errno:r;}
int sceIoRead(int fd,void *p,unsigned n){reads++;if(read_cancel && reads==read_cancel)atomic_store(&cancel,true);int r=read(fd,p,n);return r<0?-errno:r;}
int sceIoClose(int fd){return close(fd);}
int sceIoSync(const char *p,int n){(void)p;(void)n;return ++sync_count==sync_fail?-5:0;}
int sceIoRename(const char *a,const char *b){if(rename_fail)return -5;return rename(a,b);}
int sceIoGetstat(const char *p,SceIoStat *out){struct stat st;if(stat(p,&st)<0)return -errno;memset(out,0,sizeof(*out));out->st_size=st.st_size;out->st_mode=st.st_mode;return 0;}
int sceIoMkdir(const char *p,int m){return mkdir(p,m);}
int sceIoDopen(const char *p){(void)p;dir_pos=0;return dir_mode?800:-13;}
int sceIoDread(int fd,SceIoDirent *e){
 (void)fd;memset(e,0,sizeof(*e));
 if(dir_mode==1){if(dir_pos++)return 0;strcpy(e->d_name,"psp2core_example");e->d_stat.st_mode=0100000;e->d_stat.st_size=123;e->d_stat.st_mtime.year=2026;return 1;}
 if(dir_pos>=100)return 0;
 snprintf(e->d_name,sizeof(e->d_name),"file_%03d.bin",dir_pos++);e->d_stat.st_mode=0100000;return 1;
}
int sceIoDclose(int fd){(void)fd;return 0;}
int sceIoDevctl(const char *p,unsigned c,void *in,unsigned ni,void *out,unsigned no){(void)c;(void)in;(void)ni;(void)no;if(strcmp(p,"ux0:"))return -13;SceIoDevInfo *i=out;i->max_size=1000000;i->free_size=0;return 0;}
int sceMotionStartSampling(void){return device_error;}
int sceMotionStopSampling(void){return close_error;}
int sceMotionMagnetometerOn(void){return device_error;}
int sceMotionMagnetometerOff(void){return close_error;}
int sceMotionGetState(SceMotionState *m){memset(m,0,sizeof(*m));m->timestamp=123;m->hostTimestamp=456;fake_epoch++;return device_error;}
int sceAudioOutOpenPort(int a,int b,int c,int d){(void)a;(void)b;(void)c;(void)d;return device_error?device_error:3;}
int sceAudioOutOutput(int p,const void *data){(void)p;const int16_t *v=data;for(int i=0;i<512;i++)assert(abs(v[i])<=3277);fake_epoch++;return device_error;}
int sceAudioOutReleasePort(int p){(void)p;return close_error;}
int sceAudioInOpenPort(int a,int b,int c,int d){(void)a;(void)b;(void)c;(void)d;return device_error?device_error:4;}
int sceAudioInInput(int p,void *data){(void)p;memset(data,0,512);fake_epoch++;return device_error;}
int sceAudioInReleasePort(int p){(void)p;return close_error;}
int sceCameraOpen(int d,SceCameraInfo *i){(void)d;assert(i->pIBase && i->sizeIBase==VT_CAMERA_BYTES);return device_error;}
int sceCameraStart(int d){(void)d;return device_error;}
int sceCameraRead(int d,SceCameraRead *r){(void)d;r->frame=1;r->timestamp=99;fake_epoch++;return device_error;}
int sceCameraStop(int d){(void)d;return close_error;}
int sceCameraClose(int d){(void)d;return close_error;}
int sceSysmoduleLoadModule(int n){(void)n;return 0;}
int sceSysmoduleUnloadModule(int n){(void)n;return 0;}
int sceNetInit(SceNetInitParam *p){(void)p;return 0;}
int sceNetTerm(void){return 0;}
int sceNetCtlInit(void){return 0;}
void sceNetCtlTerm(void){}
int sceNetCtlInetGetState(int *s){*s=3;return device_error;}
int sceNetCtlInetGetInfo(int c,SceNetCtlInfo *i){(void)c;i->rssi_percentage=80;i->channel=6;return device_error;}
int vshMemoryCardGetCardInsertState(void){return 0;}
int vshRemovableMemoryGetCardInsertState(void){return -13;}
static void reset_test(void){epoch=fake_epoch;atomic_store(&cancel,false);focus=true;device_error=close_error=allocation_error=0;}
static void contents(const char *path,char *out,unsigned n){FILE *f=fopen(path,"r");assert(f);size_t size=fread(out,1,n-1,f);out[size]=0;fclose(f);}
int main(void){
 char temp[]="/tmp/vt-scanner-XXXXXX";assert(mkdtemp(temp));assert(chdir(temp)==0);
 assert(mkdir("ux0:data",0700)==0);assert(mkdir(ROOT,0700)==0);
 memset(&work,0,sizeof(work));vt_scan_init(&work.model,123456);reset_test();collect();
 assert(work.model.soh==75 && work.model.full==1700);assert(save());
 dir_mode=1;int before_reads=reads;dump_metadata("ux0:");assert(reads==before_reads);assert(strstr(work.model.values[work.model.count-1].value,"123 bytes"));
 dir_mode=2;browse("ux0:",0);assert(work.file_count==64 && work.files_more && !strcmp(work.files[0].name,"file_000.bin"));
 browse("ux0:",64);assert(work.file_count==36 && !work.files_more && !strcmp(work.files[0].name,"file_064.bin"));dir_mode=0;
 assert(mkdir("ur0:tai",0700)==0);FILE *cfg=fopen("ur0:tai/config.txt","w");assert(cfg);fputs("*KERNEL\nur0:tai/configured_only.skprx\n",cfg);fclose(cfg);configuration("ur0:tai/config.txt");
 assert(!strcmp(work.model.values[work.model.count-1].name,"Plugin CONFIGURADO"));assert(save());
 assert(strstr(work.report,"ALERTA: SOH") && strstr(work.report,"total=1000000 livre=0"));
 assert(strstr(work.report,"visible_only") && strstr(work.report,"Firmware real: acesso indisponivel"));
 char first[256];strcpy(first,work.report_path);
 work.report_path[0]=0;assert(save());assert(strcmp(first,work.report_path));
 assert(replace_file(ROOT "/atomic.txt","old",3)==0);
 write_fail=1;assert(replace_file(ROOT "/atomic.txt","new",3)<0);write_fail=0;
 char data[32];contents(ROOT "/atomic.txt",data,sizeof(data));assert(!strcmp(data,"old"));
 sync_count=0;sync_fail=1;assert(replace_file(ROOT "/atomic.txt","new",3)<0);contents(ROOT "/atomic.txt",data,sizeof(data));assert(!strcmp(data,"old"));sync_fail=0;
 rename_fail=1;assert(!save());assert(work.save_failed && work.report[0]);rename_fail=0;assert(save());
 sync_count=0;sync_fail=2;assert(replace_file(ROOT "/atomic.txt","new",3)<0);sync_fail=0;
 assert(replace_file(ROOT "/atomic.txt","complete",8)==0);contents(ROOT "/atomic.txt",data,sizeof(data));assert(!strcmp(data,"complete"));
 for(int i=0;i<10;i++) {reset_test();run_test(i,"");assert(work.model.tests[i].verdict==VT_PENDING);assert(!allocation && camera_device<0 && audio_port<0 && mic_port<0 && !motion_on);}
 assert(!work.model.tests[9].available);assert(!vt_scan_confirm(&work.model,9,VT_PASS,1));assert(vt_scan_confirm(&work.model,9,VT_SKIP,1));
 reset_test();device_error=-5;run_test(3,"");assert(work.model.tests[3].error==-5 && !work.model.tests[3].available);
 reset_test();allocation_error=1;run_test(6,"");assert(work.model.tests[6].error==-12 && !allocation);
 reset_test();close_error=-5;run_test(6,"");assert(camera_device>=0 && allocation);assert(!release_devices());close_error=0;assert(release_devices());assert(!allocation);
 reset_test();epoch=fake_epoch-1;run_test(5,"");assert(!work.model.tests[5].available);
 assert(mkdir("ux0:",0700)==0);FILE *f=fopen("ux0:/read.bin","w");assert(f);for(int i=0;i<300000;i++)fputs("secret",f);fclose(f);
 reset_test();reads=0;read_cancel=0;run_test(10,"ux0:/read.bin");assert(strstr(work.model.tests[10].detail,"1048576 bytes"));assert(reads==256);
 reset_test();reads=0;read_cancel=2;run_test(10,"ux0:/read.bin");assert(reads==2 && strstr(work.model.tests[10].detail,"cancelada"));
 reset_test();run_test(10,"ux0:/psp2core.dump");assert(!work.model.tests[10].available);
 reset_test();run_test(10,"ux0:/missing");assert(work.model.tests[10].error<0);
 assert(save());assert(!strstr(work.report,"secret") && !strstr(work.report,"SSID") && !strstr(work.report,"BSSID"));
 puts("scanner production worker: partial inventory, collision, short write/sync/rename, device errors, cleanup retry, epoch cancellation, 1 MiB read, privacy OK");
}
