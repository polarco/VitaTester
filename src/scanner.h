#ifndef VT_SCANNER_H
#define VT_SCANNER_H
#include "scan_model.h"
#include <stdatomic.h>
#define VT_SCAN_FILES 64
#define VT_CAMERA_BYTES (160*120*4)
typedef struct {char name[256];bool directory;} VtScanFile;
typedef struct {
 VtScan model;char report[VT_REPORT_SIZE],status[256],path[512],report_path[256];
 VtScanFile files[VT_SCAN_FILES];unsigned file_count,file_offset;bool files_more,busy,save_failed,finished,preview;
 unsigned pixels[160*120];unsigned frame;int active;
} VtScannerView;
enum {SCAN_COLLECT=1,SCAN_TEST,SCAN_VERDICT,SCAN_SETTINGS,SCAN_BROWSE,SCAN_SAVE,SCAN_COMPLETE,SCAN_EXIT};
int vt_scanner_enter(void);
void vt_scanner_snapshot(VtScannerView *);
bool vt_scanner_request(int job,int test,int value,const char *path);
void vt_scanner_cancel(void);
bool vt_scanner_leave(void);
#endif
