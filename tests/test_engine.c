#include "break174.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static b174_engine a,b;
static int32_t x[512],y[512];
static uint32_t hash(uint32_t h,const int32_t *p,unsigned n){while(n--)h=(h^(uint32_t)*p++)*16777619u;return h;}
static void initial_and_audio(void){
 b174_init(&a,44100,174);
 assert(b174_status(&a,B174_S_BPM)==174);
 assert(!b174_status(&a,B174_S_RUNNING));
 b174_command(&a,B174_C_PLAY,1); b174_init(&b,44100,174); b174_command(&b,B174_C_PLAY,1);
 uint64_t energy=0;uint32_t hx=2166136261u,hy=hx;
 for(int n=0;n<400;n++){b174_render(&a,x,256);b174_render(&b,y,256);hx=hash(hx,x,512);hy=hash(hy,y,512);for(int i=0;i<512;i++){assert(x[i]>=-32767&&x[i]<=32767);energy+=(uint64_t)(x[i]<0?-x[i]:x[i]);}}
 assert(energy>1000000);assert(hx==hy);assert(b174_status(&a,B174_S_FRAME)==102400);
}
static void bounded_settings(void){
 b174_init(&a,44100,1);b174_command(&a,B174_C_BPM,0);assert(b174_status(&a,B174_S_BPM)==140);
 b174_command(&a,B174_C_BPM,999);assert(b174_status(&a,B174_S_BPM)==190);
 b174_command(&a,999,999);b174_render(&a,x,256);
}
static void chunk_independent(void){
 b174_init(&a,44100,42);b174_init(&b,44100,42);b174_command(&a,B174_C_PLAY,1);b174_command(&b,B174_C_PLAY,1);
 for(int j=0;j<200;j++){b174_render(&a,x,256);for(int k=0;k<256;k++)b174_render(&b,y+2*k,1);assert(!memcmp(x,y,sizeof x));}
}
static void performance_phase_and_print(void){
 const int tempos[]={140,166,174,190};
 for(unsigned ti=0;ti<sizeof tempos/sizeof tempos[0];ti++){int tempo=tempos[ti];
 b174_init(&a,44100,52);b174_init(&b,44100,52);b174_command(&a,B174_C_BPM,tempo);b174_command(&b,B174_C_BPM,tempo);
 b174_command(&a,B174_C_PLAY,1);b174_command(&b,B174_C_PLAY,1);
 for(int n=0;n<800;n++){
  if(n==150){b174_key(&a,1,2,1);b174_key(&a,1,4,1);}
  if(n==300){b174_key(&a,1,2,0);b174_key(&a,1,4,0);}
  if(n==400)b174_key(&a,0,11,1);
  b174_render(&a,x,256);b174_render(&b,y,256);
  assert(a.frame==b.frame&&a.step==b.step&&a.bar==b.bar);
  assert(a.write_count<=B174_PCM&&a.pcm_length[a.ready_bank]<=B174_PCM);
 }
 assert(a.print_ready);assert(!a.held);assert(a.pcm_length[a.ready_bank]>27000);
 }
}
static void pcm_rate(void){
 b174_init(&a,44100,4);b174_command(&a,B174_C_PLAY,1);
 for(int n=0;n<300;n++)b174_render(&a,x,256);
 assert(a.print_ready);uint32_t old=a.read_position;
 b174_render(&a,x,100);assert(a.read_position-old==50); /* 22.05 kHz PCM into 44.1 kHz output */
 b174_key(&a,1,5,1);old=a.read_position;b174_render(&a,x,100);assert(a.read_position-old==25);
 b174_key(&a,1,5,0);b174_key(&a,1,7,1);old=a.read_position;b174_render(&a,x,100);assert(a.read_position-old==25);
}
static void stop_release_and_return(void){
 b174_init(&a,44100,4);b174_command(&a,B174_C_PLAY,1);b174_key(&a,1,1,1);b174_command(&a,B174_C_COMPLEXITY,100);
 b174_command(&a,B174_C_RETURN,0);assert(!a.held&&a.set.complexity==24);
 b174_command(&a,B174_C_PLAY,0);for(int n=0;n<40;n++)b174_render(&a,x,256);
 for(int i=0;i<512;i++)assert(x[i]==0);
 b174_key(&a,1,-1,1);b174_key(&a,1,50,1);assert(!a.held);
}
static void fresh_restart(void){
 b174_init(&a,44100,4);b174_command(&a,B174_C_PLAY,1);for(int n=0;n<300;n++)b174_render(&a,x,256);
 b174_command(&a,B174_C_FILL,16);b174_key(&a,0,13,1);b174_key(&a,1,2,1);
 b174_command(&a,B174_C_PLAY,0);b174_command(&a,B174_C_PLAY,1);
 assert(!a.fill_until&&!a.drum_mute&&!a.held&&!a.roll_at&&!a.print_ready&&a.read_position==0);
}
static void stopped_audition(void){
 b174_init(&a,44100,1);b174_hit(&a,B174_SNARE,110);b174_render(&a,x,256);uint32_t energy=0;for(int i=0;i<512;i++)energy+=(uint32_t)(x[i]<0?-x[i]:x[i]);assert(energy>10000);assert(!a.running&&a.frame==0);
}
static void early_fill_and_kill_priority(void){
 b174_init(&a,44100,17);b174_init(&b,44100,17);b174_command(&a,B174_C_PLAY,1);b174_command(&b,B174_C_PLAY,1);b174_render(&a,x,256);b174_render(&b,y,256);b174_command(&a,B174_C_FILL,4);int different=0;
 for(int n=0;n<100;n++){b174_render(&a,x,256);b174_render(&b,y,256);different|=memcmp(x,y,sizeof x)!=0;}assert(different);
 b174_init(&a,44100,1);b174_command(&a,B174_C_BASS,0);b174_command(&a,B174_C_PLAY,1);b174_render(&a,x,256);b=a;
 b174_key(&a,1,10,1);b174_key(&b,1,10,1);b174_key(&b,1,9,1);b174_render(&a,x,256);b174_render(&b,y,256);assert(!memcmp(x,y,sizeof x));
}
static void bass_off_is_silent(void){
 b174_init(&a,44100,1);b174_command(&a,B174_C_PLAY,1);b174_render(&a,x,256);b174_command(&a,B174_C_BASS,0);for(int i=0;i<150;i++)b174_render(&a,x,256);assert(a.bass_env==0);
}
static void dub_tail_and_panic(void){
 b174_init(&a,44100,17);b174_init(&b,44100,17);b174_key(&a,1,8,1);
 b174_hit(&a,B174_SNARE,120);b174_hit(&b,B174_SNARE,120);
 for(int i=0;i<10;i++){b174_render(&a,x,256);b174_render(&b,y,256);}
 b174_key(&a,1,8,0);int different=0;
 for(int i=0;i<100;i++){b174_render(&a,x,256);b174_render(&b,y,256);different|=memcmp(x,y,sizeof x)!=0;}
 assert(different);b174_command(&a,B174_C_PANIC,0);b174_render(&a,x,256);for(int i=0;i<512;i++)assert(x[i]==0);
 b174_command(&b,B174_C_BASS,0);b174_key(&b,1,10,1);b174_render(&b,y,256);for(int i=0;i<512;i++)assert(y[i]==0);
}
static void styles_are_distinct(void){
 uint32_t h[3];for(int style=0;style<3;style++){b174_init(&a,44100,174);b174_command(&a,B174_C_STYLE,style);b174_command(&a,B174_C_PLAY,1);h[style]=2166136261u;for(int n=0;n<900;n++){b174_render(&a,x,256);h[style]=hash(h[style],x,512);}}
 assert(h[0]!=h[1]&&h[1]!=h[2]&&h[0]!=h[2]);
}
int main(void){initial_and_audio();bounded_settings();chunk_independent();performance_phase_and_print();pcm_rate();stop_release_and_return();fresh_restart();stopped_audition();early_fill_and_kill_priority();bass_off_is_silent();dub_tail_and_panic();styles_are_distinct();puts("engine: 12 tests passed");}
