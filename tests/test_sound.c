#include "break174.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static b174_engine a,b;
static int32_t x[512],y[512];
static void advance(b174_engine *e,unsigned frames){while(frames){unsigned n=frames>256?256:frames;b174_render(e,x,n);frames-=n;}}
int main(void){
 /* Musical layer must be audible independently and disappear when its level is zero. */
 b174_init(&a,44100,174);b174_init(&b,44100,174);
 b174_command(&a,B174_C_BASS,0);b174_command(&b,B174_C_BASS,0);
 b174_command(&a,19,80);b174_command(&b,19,0);
 b174_command(&a,B174_C_PLAY,1);b174_command(&b,B174_C_PLAY,1);
 b174_key(&a,1,10,1);b174_key(&b,1,10,1);
 uint64_t energy=0;for(int i=0;i<350;i++){b174_render(&a,x,256);b174_render(&b,y,256);for(int k=0;k<512;k++){energy+=(uint64_t)(x[k]<0?-x[k]:x[k]);assert(y[k]==0);}}
 assert(energy>1000000);
 b174_command(&a,B174_C_PANIC,0);b174_render(&a,x,256);for(int k=0;k<512;k++)assert(x[k]==0);
 /* Sub phrases contain an actual release/rest, rather than an unbroken oscillator. */
 b174_init(&a,44100,174);b174_command(&a,19,0);b174_command(&a,B174_C_PLAY,1);
 unsigned step=a.step_q16>>16;advance(&a,step);assert(a.bass_env>2000);
 advance(&a,step*9/2);assert(a.bass_env<1500);
 /* No stuck cymbal/voice or out-of-bounds reads at the slowest and fastest tempo. */
 for(int bpm=140;bpm<=190;bpm+=50){b174_init(&a,44100,5);b174_command(&a,B174_C_BPM,bpm);for(int role=0;role<B174_VOICES;role++)b174_hit(&a,role,127);advance(&a,44100*3);for(int role=0;role<B174_VOICES;role++)assert(a.voice[role].env==0);}
 puts("sound: independent atmosphere, panic, bass rests and voice tails passed");
}
