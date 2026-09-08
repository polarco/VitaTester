#ifndef VT_RUNTIME_H
#define VT_RUNTIME_H
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/types.h>
#include <stdint.h>
#include <stdbool.h>
#include "diagnostic.h"
#define VT_CAPTURE_PRIORITY 64
#define VT_UI_PRIORITY 80
#define VT_LOG_PRIORITY 96
#define VT_WORK_PRIORITY 160
typedef struct {
    SceCtrlData pad;
    SceTouchData front,back;
    int rc[3], fresh[3], clocks[3], temp, thermal_rc, battery, online;
    uint64_t now, thermal_at, elapsed, max_poll, p99_poll;
    unsigned confirmed, write_ms, queue_delay_ms, workers[3];
    int priorities[6], affinities[3];
    bool running, log_failed, restore_failed, priority_ok, valid;
    VtDiagnostic diagnostic;
    unsigned setting;
    char status[100];
} VtSnapshot;
int vt_runtime_init(void);
void vt_runtime_snapshot(VtSnapshot *);
void vt_runtime_shutdown(void);
void vt_runtime_fps(unsigned);
// Logger and workers communicate only through atomics, no vita2d access.
int vt_logger_start(void);
SceUID vt_logger_id(void);
int vt_logger_state(void);
void vt_logger_finish(void);
void vt_log_status(unsigned *,unsigned *,unsigned *);
bool vt_log_enqueue(const char *,unsigned,unsigned,uint64_t);
int vt_workers_start(void);
SceUID vt_worker_id(int);
unsigned vt_worker_progress(int);
void vt_workers_enable(bool);
void vt_workers_finish(void);
#endif
