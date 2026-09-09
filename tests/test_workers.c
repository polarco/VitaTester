#include "../src/workers.c"
#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
static pthread_t host_threads[3];
static unsigned created;
static void *run(void *arg){worker(sizeof(int),arg);free(arg);return NULL;}
int sceKernelCreateThread(const char *n,int (*f)(SceSize,void *),int priority,unsigned stack,unsigned attr,int affinity,void *opt){(void)n;(void)f;(void)priority;(void)stack;(void)attr;(void)opt;assert(created<3);if(affinity!=(0x10000<<(int)created))return -999;return (int)created++;}
int sceKernelStartThread(int id,unsigned size,void *arg){(void)size;int *index=malloc(sizeof(int));assert(index);*index=*(int *)arg;return pthread_create(&host_threads[id],NULL,run,index);}
int sceKernelDelayThread(unsigned us){struct timespec t={.tv_sec=us/1000000,.tv_nsec=(us%1000000)*1000};return nanosleep(&t,NULL);}
int sceKernelWaitThreadEnd(int id,void *status,unsigned *timeout){(void)status;(void)timeout;return pthread_join(host_threads[id],NULL);}
int main(void){
 assert(vt_workers_start()==0);
 for(int cycle=0;cycle<20;cycle++) {
  unsigned old[3];for(int i=0;i<3;i++)old[i]=vt_worker_progress(i);
  vt_workers_enable(true);
  for(int i=0;i<3;i++){int attempts=0;while(vt_worker_progress(i)==old[i] && attempts++<1000)sceKernelDelayThread(1000);assert(attempts<1000);}
  vt_workers_enable(false);
  int attempts=0;while(!vt_workers_idle() && attempts++<1000)sceKernelDelayThread(1000);assert(attempts<1000);
  unsigned stopped_at[3];for(int i=0;i<3;i++)stopped_at[i]=vt_worker_progress(i);
  sceKernelDelayThread(10000);
  for(int i=0;i<3;i++)assert(stopped_at[i]==vt_worker_progress(i));
 }
 vt_workers_finish();assert(vt_workers_idle());puts("production workers: concurrent drain acknowledgement and no load after module exit OK");
}
