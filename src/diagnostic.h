#ifndef VT_DIAGNOSTIC_H
#define VT_DIAGNOSTIC_H
#include <stdint.h>
#include <stdbool.h>
#define VT_BUTTONS 12
#define VT_SECOND UINT64_C(1000000)
extern const uint32_t vt_masks[VT_BUTTONS];
extern const char *vt_names[VT_BUTTONS];
typedef void (*VtEmit)(void *, const char *, const char *, const char *, uint64_t, uint64_t);
typedef struct {
    uint64_t held, idle, last_good;
    bool down, pressed, cycled, held_alert, idle_alert;
} VtButton;
typedef struct {
    uint64_t now;
    uint32_t buttons;
    int axes[4], rear_count, rear_x, rear_y;
    bool valid, ctrl_new, rear_new, front_activity;
} VtInput;
typedef struct {
    unsigned held_s, idle_s, step_s, ghost_s, rear_s;
} VtLimits;
typedef struct {
    VtLimits limits;
    VtButton button[VT_BUTTONS];
    uint64_t previous, rear_time, rear_good, step_time, ghost_time;
    uint64_t independent[VT_BUTTONS];
    int axes[4], rear_x, rear_y, rear_count, step;
    bool initialized, valid, guided, step_pressed, step_success, ghost_alert, rear_alert, missing_alert, step_contact;
    VtEmit emit;
    void *opaque;
} VtDiagnostic;
void vt_diag_init(VtDiagnostic *, VtEmit, void *);
void vt_diag_mode(VtDiagnostic *, bool, uint64_t);
void vt_diag_feed(VtDiagnostic *, const VtInput *);
const char *vt_step_name(const VtDiagnostic *);
#endif
