#include "scan_model.h"
#include "navigation.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static VtScan scan;
static char report[VT_REPORT_SIZE];
int main(void) {
 vt_scan_init(&scan,100);assert(vt_scan_health(&scan)<0);
 scan.battery=true;scan.full=1800;scan.remaining=1000;scan.soh=79;
 assert(vt_scan_health(&scan)<0);scan.nominal=0;assert(vt_scan_health(&scan)<0);
 scan.nominal=2000;assert(vt_scan_health(&scan)==90);
 assert(vt_scan_report(&scan,report,sizeof(report)));assert(strstr(report,"ALERTA: SOH"));assert(!strstr(report,"ALERTA: saude calculada"));
 scan.soh=80;assert(vt_scan_report(&scan,report,sizeof(report)));assert(!strstr(report,"ALERTA: SOH"));
 scan.full=1599;assert(vt_scan_health(&scan)<80);scan.full=1600;assert(vt_scan_health(&scan)==80);
 scan.remaining=1601;assert(vt_scan_health(&scan)<0);scan.remaining=-1;assert(vt_scan_health(&scan)<0);
 scan.remaining=0;assert(vt_scan_health(&scan)==80);scan.full=0;assert(vt_scan_health(&scan)<0);
 scan.full=-1;assert(vt_scan_health(&scan)<0);scan.full=10001;assert(vt_scan_health(&scan)<0);
 scan.full=1800;scan.battery=false;assert(vt_scan_health(&scan)<0);
 scan.battery=true;scan.nominal=100;assert(vt_scan_health(&scan)<0);
 vt_scan_add(&scan,"montagem","acesso negado","rc","sceIoDevctl",VT_ERROR,false,101);
 assert(!vt_scan_confirm(&scan,0,VT_PASS,102));assert(!vt_scan_complete(&scan));
 for(unsigned i=0;i<VT_SCAN_TESTS;i++)assert(vt_scan_confirm(&scan,i,VT_SKIP,102));
 assert(vt_scan_complete(&scan));assert(vt_scan_report(&scan,report,sizeof(report)));assert(strstr(report,"CONCLUIDO"));assert(strstr(report,"PULADO"));assert(strstr(report,"acesso negado"));
 scan.tests[0].attempted=scan.tests[0].available=true;scan.tests[0].error=-5;
 assert(vt_scan_confirm(&scan,0,VT_PASS,104));assert(scan.tests[0].error==-5);
 assert(vt_scan_confirm(&scan,0,VT_FAIL,105));assert(vt_scan_report(&scan,report,sizeof(report)));assert(strstr(report,"FALHOU: Acelerometro"));
 assert(!vt_scan_report(&scan,report,16));
 VtGesture g={0};assert(!vt_gesture(&g,1,true,0,0));
 for(uint64_t t=10000;t<=2000000;t+=10000)assert(!vt_gesture(&g,t,true,3,3));
 assert(vt_gesture(&g,2010000,true,3,3));assert(!vt_gesture(&g,2020000,true,3,3));
 assert(!vt_gesture(&g,2030000,true,0,0));
 assert(!vt_gesture(&g,2040000,true,3,3));
 assert(!vt_gesture(&g,2100000,true,3,3));assert(g.latched);
 assert(!vt_gesture(&g,2110000,true,0,0));assert(!g.latched);
 assert(!vt_gesture(&g,2120000,false,0,0));assert(g.latched);
 assert(!vt_gesture(&g,2130000,true,3,3));assert(g.latched);
 assert(!vt_gesture(&g,2140000,true,0,0));
 for(uint64_t t=2150000;t<5000000;t+=10000)assert(!vt_gesture(&g,t,true,3,2));
 puts("scanner model/navigation: health, absent battery, errors, threshold, verdicts, partial/full, overflow, gesture/focus/gaps OK");
}
