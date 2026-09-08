#ifndef VT_SCAN_MODEL_H
#define VT_SCAN_MODEL_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define VT_SCAN_TESTS 11
#define VT_SCAN_ROWS 220
#define VT_SCAN_TEXT 256
#define VT_REPORT_SIZE 160000
typedef enum {VT_AVAILABLE,VT_UNAVAILABLE,VT_ERROR} VtAvailability;
typedef enum {VT_PENDING,VT_PASS,VT_FAIL,VT_SKIP} VtVerdict;
typedef struct {
 char name[64],value[VT_SCAN_TEXT],unit[24],source[80];
 int64_t at;VtAvailability availability;bool alert;
} VtScanValue;
typedef struct {VtVerdict verdict;bool attempted,available;int error;unsigned attempts;int64_t at;char detail[VT_SCAN_TEXT];} VtScanTest;
typedef struct {
 int nominal,threshold,soh,full,remaining;bool battery;
 int64_t started,ended;bool complete;unsigned count;
 VtScanValue values[VT_SCAN_ROWS];VtScanTest tests[VT_SCAN_TESTS];
} VtScan;
extern const char *const vt_scan_names[VT_SCAN_TESTS];
void vt_scan_init(VtScan *,int64_t);
void vt_scan_add(VtScan *,const char *,const char *,const char *,const char *,VtAvailability,bool,int64_t);
double vt_scan_health(const VtScan *);
bool vt_scan_confirm(VtScan *,unsigned,VtVerdict,int64_t);
bool vt_scan_complete(VtScan *);
size_t vt_scan_report(const VtScan *,char *,size_t);
#endif
