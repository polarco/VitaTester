#ifndef VT_FAKE_VITA_H
#define VT_FAKE_VITA_H
#include <stdint.h>
#include <stddef.h>
typedef int SceUID;
typedef unsigned SceSize;
typedef unsigned SceUInt;
typedef struct {uint64_t timeStamp;unsigned buttons;unsigned char lx,ly,rx,ry;} SceCtrlData;
#define SCE_TOUCH_MAX_REPORT 8
typedef struct {unsigned short x,y;unsigned char id;} SceTouchReport;
typedef struct {uint64_t timeStamp;unsigned reportNum;SceTouchReport report[8];} SceTouchData;
typedef struct {unsigned systemEventNum;int isSystemUiOverlaid;} SceAppMgrAppState;
typedef struct {int systemEvent;} SceAppMgrSystemEvent;
typedef struct {unsigned size;int currentPriority,currentCpuAffinityMask;} SceKernelThreadInfo;
#define SCE_APPMGR_SYSTEMEVENT_ON_RESUME 0x10000003
#define SCE_POWER_CB_SYSTEM_SUSPEND 0x10000
#define SCE_POWER_CB_SYSTEM_RESUMING 0x20000
#define SCE_POWER_CB_SYSTEM_RESUME 0x40000
#define SCE_POWER_CB_APP_RESUME 0x200000
#define SCE_POWER_CB_APP_SUSPEND 0x400000
#define SCE_POWER_CB_APP_RESUMING 0x800000
#define SCE_CTRL_MODE_ANALOG_WIDE 2
#define SCE_TOUCH_SAMPLING_STATE_START 1
#define SCE_TOUCH_PORT_FRONT 0
#define SCE_TOUCH_PORT_BACK 1
int scePowerGetArmClockFrequency(void);
int scePowerGetGpuClockFrequency(void);
int scePowerGetBusClockFrequency(void);
int scePowerSetArmClockFrequency(int);
int scePowerSetGpuClockFrequency(int);
int scePowerSetBusClockFrequency(int);
int scePowerGetBatteryTemp(void);
int scePowerGetBatteryLifePercent(void);
int scePowerIsPowerOnline(void);
int scePowerIsSuspendRequired(void);
int scePowerRegisterCallback(int);
int scePowerUnregisterCallback(int);
int sceKernelCreateCallback(const char *,int,int (*)(int,int,int,void *),void *);
int sceKernelDeleteCallback(int);
int sceKernelCheckCallback(void);
int sceKernelGetThreadInfo(int,SceKernelThreadInfo *);
int sceKernelGetThreadId(void);
int sceKernelChangeThreadPriority(int,int);
int sceKernelCreateThread(const char *,int (*)(SceSize,void *),int,unsigned,unsigned,int,void *);
int sceKernelStartThread(int,unsigned,void *);
int sceKernelWaitThreadEnd(int,void *,unsigned *);
int sceKernelCreateMutex(const char *,unsigned,int,void *);
int sceKernelTryLockMutex(int,int);
int sceKernelUnlockMutex(int,int);
int sceKernelDelayThreadCB(unsigned);
uint64_t sceKernelGetProcessTimeWide(void);
int _sceAppMgrGetAppState(SceAppMgrAppState *,size_t,unsigned);
int sceAppMgrReceiveSystemEvent(SceAppMgrSystemEvent *);
int sceCtrlGetButtonIntercept(int *);
int sceCtrlSetSamplingMode(int);
int sceTouchSetSamplingState(int,int);
int sceCtrlPeekBufferPositive(int,SceCtrlData *,int);
int sceTouchPeek(int,SceTouchData *,unsigned);
#endif
