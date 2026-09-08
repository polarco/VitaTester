#ifndef VT_SCANNER_VITA_H
#define VT_SCANNER_VITA_H
#include "vita.h"
#include <sys/stat.h>
#undef st_mtime
typedef unsigned SceSize;
typedef unsigned SceUInt;
typedef struct {unsigned year,month,day,hour,minute,second;} Date;
typedef struct {unsigned st_mode;uint64_t st_size;Date st_mtime;} SceIoStat;
typedef struct {char d_name[256];SceIoStat d_stat;} SceIoDirent;
typedef struct {uint64_t max_size,free_size;} SceIoDevInfo;
typedef struct {unsigned size;char versionString[28];} SceKernelSystemSwVersion;
typedef struct {unsigned size;int size_user,size_cdram,size_phycont;} SceKernelFreeMemorySizeInfo;
typedef struct {unsigned size;char module_name[28];} SceKernelModuleInfo;
typedef struct {float x,y,z;} SceFVector3;
typedef struct {SceFVector3 acceleration,angularVelocity;unsigned timestamp;uint64_t hostTimestamp;float nedMatrix[16];unsigned magFieldStability;} SceMotionState;
typedef struct {unsigned size,priority,format,resolution,framerate,sizeIBase;void *pIBase;} SceCameraInfo;
typedef struct {unsigned size,mode,sizeIBase;void *pIBase;uint64_t frame,timestamp;} SceCameraRead;
typedef struct {void *memory;unsigned size,flags;} SceNetInitParam;
typedef struct {unsigned rssi_percentage,channel;} SceNetCtlInfo;
#define SCE_S_ISREG S_ISREG
#define SCE_S_ISDIR S_ISDIR
#define SCE_O_RDONLY 1
#define SCE_O_WRONLY 2
#define SCE_O_CREAT 4
#define SCE_O_TRUNC 8
#define SCE_O_EXCL 16
#define SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW 0
#define SCE_CAMERA_DEVICE_FRONT 0
#define SCE_CAMERA_DEVICE_BACK 1
#define SCE_CAMERA_PRIORITY_SHARE 0
#define SCE_CAMERA_FORMAT_ABGR 5
#define SCE_CAMERA_RESOLUTION_160_120 3
#define SCE_CAMERA_FRAMERATE_30_FPS 30
#define SCE_CAMERA_ERROR_ALREADY_READ (-20)
#define SCE_CAMERA_ERROR_TIMEOUT (-21)
#define SCE_AUDIO_OUT_PORT_TYPE_MAIN 0
#define SCE_AUDIO_OUT_MODE_STEREO 0
#define SCE_AUDIO_IN_PORT_TYPE_VOICE 0
#define SCE_AUDIO_IN_PARAM_FORMAT_S16_MONO 0
#define SCE_SYSMODULE_NET 0
#define SCE_NETCTL_INFO_GET_RSSI_PERCENTAGE 1
#define SCE_NETCTL_INFO_GET_CHANNEL 2
int sceKernelLockMutex(int,int,void *);
int sceKernelDeleteMutex(int);
int sceKernelDeleteThread(int);
int sceKernelDelayThread(unsigned);
int sceKernelAllocMemBlock(const char *,int,unsigned,void *);
int sceKernelGetMemBlockBase(int,void **);
int sceKernelFreeMemBlock(int);
int sceKernelIsPSVitaTV(void);
int scePowerGetBatterySOH(void);
int scePowerGetBatteryFullCapacity(void);
int scePowerGetBatteryRemainCapacity(void);
int scePowerGetBatteryVolt(void);
int sceKernelGetSystemSwVersion(SceKernelSystemSwVersion *);
int _vshSblGetSystemSwVersion(SceKernelSystemSwVersion *);
int sceKernelGetFreeMemorySize(SceKernelFreeMemorySizeInfo *);
int _vshKernelSearchModuleByName(const char *,const void *);
int sceKernelGetModuleList(unsigned,SceUID *,SceSize *);
int sceKernelGetModuleInfo(int,SceKernelModuleInfo *);
int sceIoOpen(const char *,int,int);
int sceIoWrite(int,const void *,unsigned);
int sceIoRead(int,void *,unsigned);
int sceIoClose(int);
int sceIoSync(const char *,int);
int sceIoRename(const char *,const char *);
int sceIoGetstat(const char *,SceIoStat *);
int sceIoMkdir(const char *,int);
int sceIoDopen(const char *);
int sceIoDread(int,SceIoDirent *);
int sceIoDclose(int);
int sceIoDevctl(const char *,unsigned,void *,unsigned,void *,unsigned);
int sceMotionStartSampling(void);
int sceMotionStopSampling(void);
int sceMotionMagnetometerOn(void);
int sceMotionMagnetometerOff(void);
int sceMotionGetState(SceMotionState *);
int sceAudioOutOpenPort(int,int,int,int);
int sceAudioOutOutput(int,const void *);
int sceAudioOutReleasePort(int);
int sceAudioInOpenPort(int,int,int,int);
int sceAudioInInput(int,void *);
int sceAudioInReleasePort(int);
int sceCameraOpen(int,SceCameraInfo *);
int sceCameraStart(int);
int sceCameraRead(int,SceCameraRead *);
int sceCameraStop(int);
int sceCameraClose(int);
int sceSysmoduleLoadModule(int);
int sceSysmoduleUnloadModule(int);
int sceNetInit(SceNetInitParam *);
int sceNetTerm(void);
int sceNetCtlInit(void);
void sceNetCtlTerm(void);
int sceNetCtlInetGetState(int *);
int sceNetCtlInetGetInfo(int,SceNetCtlInfo *);
int vshMemoryCardGetCardInsertState(void);
int vshRemovableMemoryGetCardInsertState(void);
#endif
