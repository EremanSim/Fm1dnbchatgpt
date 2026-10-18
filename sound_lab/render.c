/* SPDX-License-Identifier: GPL-3.0-only
 * Native audition adapter. Full 44.1 kHz rendering; no proprietary SDK. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "drum808_state.h"
#include "bass303_state.h"
#include "synth_dx7.h"
#define RATE 44100
#define FRAMES (RATE*26)
#define BLOCK 128
void groove_fm_program(uint8_t patch[156],unsigned program);
typedef struct {unsigned frame;int role,velocity,off;} event;
static event events[1024];static unsigned count,cursor;
static drum808_t kit[2];static bass303_t bass;static synth_dx7_t *fm;static uint8_t patch[156];
static unsigned round_robin[D8_NUM];
static void add(double seconds,int role,int velocity,int off){if(count>=1024)exit(3);events[count++]=(event){(unsigned)(seconds*RATE+.5),role,velocity,off};}
static int by_time(const void *a,const void *b){const event *x=a,*y=b;return x->frame<y->frame?-1:x->frame>y->frame?1:x->off>y->off?-1:x->off<y->off?1:0;}
static void drum_param(int part,const char *name,int value){for(int i=0;i<drum808_nparams(part);i++)if(!strcmp(drum808_param(part,i)->name,name))for(int k=0;k<2;k++)drum808_set(&kit[k],part,i,value);}
static void drums_setup(void){
 for(int k=0;k<2;k++)drum808_init(&kit[k]);
 drum_param(D8_BD,"Decay",52);drum_param(D8_BD,"Tune",55);drum_param(D8_BD,"Attack",18);
 drum_param(D8_SD,"Decay",83);drum_param(D8_SD,"Tune",59);drum_param(D8_SD,"Snappy",92);drum_param(D8_SD,"Tone",78);
 drum_param(D8_CH,"Decay",42);drum_param(D8_CH,"Level",39);drum_param(D8_CH,"Pan",49);
 drum_param(D8_OH,"Decay",48);drum_param(D8_OH,"Level",32);drum_param(D8_OH,"Pan",77);
 drum_param(D8_CY,"Decay",56);drum_param(D8_CY,"Tune",53);drum_param(D8_CY,"Level",24);drum_param(D8_CY,"Pan",78);
 for(int k=0;k<2;k++)drum808_pan_settle(&kit[k]);
 add(.08,D8_BD,106,0);add(.75,D8_SD,105,0);add(1.45,D8_CH,84,0);add(2.0,D8_CY,70,0);add(2.7,D8_SD,38,0);add(3.1,D8_SD,68,0);add(3.5,D8_SD,106,0);
 double step=60.0/166/4;
 for(int bar=0;bar<12;bar++)for(int s=0;s<16;s++){
  double t=4+(bar*16+s)*step;
  if(s==0||s==6||s==(bar%2?11:10))add(t,D8_BD,s==0?108:87,0);
  if(s==4||s==12)add(t+.0017,D8_SD,s==4?104:97,0);
  if(s==3||s==7||s==15)add(t+step*(s==15?.5:.1),D8_SD,s==7?35:27,0);
  if(!(s&1))add(t+(s&2?.002:0),D8_CH,s&2?51:72,0);
  if((s==5||s==13)&&bar&1)add(t+.006,D8_CH,30,0);
  if((s==2||s==10)&&!(bar&1))add(t+.003,D8_CY,43,0);
  if(s==14&&bar%4==3)add(t+.006,D8_OH,67,0);
 }
}
static void bass_setup(void){
 bass303_init(&bass);bass303_set(&bass,BASS303_CUTOFF,18);bass303_set(&bass,BASS303_RESO,7);
 bass303_set(&bass,BASS303_ENVMOD,12);bass303_set(&bass,BASS303_DECAY,72);bass303_set(&bass,BASS303_ACCENT,20);
 bass303_set(&bass,BASS303_VOLUME,110);bass303_set(&bass,BASS303_DRIVE,0);bass303_set(&bass,BASS303_SLIDE,28);
 const int note[4]={29,37,32,39};
 add(.08,29,92,0);add(1.1,29,0,1);add(1.65,41,96,0);add(2.35,41,0,1);add(2.9,29,92,0);add(3.2,36,92,0);add(3.5,36,0,1);
 double step=60.0/166/4;
 for(int bar=0;bar<12;bar++){int root=note[(bar/2)&3];for(int n=0;n<3;n++){int s=n==0?0:n==1?6:10;double t=4+(bar*16+s)*step;add(t,root+(n==1&&bar&1?7:0),96,0);add(t+step*(n==1?1.7:2.6),root,0,1);}}
}
static void keys_setup(void){
 fm=synth_dx7_create(16,RATE,BLOCK);if(!fm)exit(4);groove_fm_program(patch,6);
 patch[139]=patch[140]=0; /* No pitch/amp LFO for this dry timbre audition. */
 add(.08,53,51,0);add(1.15,53,0,1);add(2,53,106,0);add(3.15,53,0,1);
 const int chord[4][4]={{56,60,63,67},{53,56,60,63},{55,58,60,63},{55,58,60,65}};
 for(int c=0;c<6;c++){double t=4+c*2.9;for(int v=0;v<4;v++){add(t+.014*v,chord[c&3][v],(c&1?76:88)-v*3,0);add(t+1.85,chord[c&3][v],0,1);}}
}
int main(int argc,char **argv){
 if(argc!=3){fprintf(stderr,"render-lab drums|bass|keys output.f32\n");return 2;}
 int mode=!strcmp(argv[1],"drums")?0:!strcmp(argv[1],"bass")?1:!strcmp(argv[1],"keys")?2:-1;if(mode<0)return 2;
 if(mode==0)drums_setup();else if(mode==1)bass_setup();else keys_setup();qsort(events,count,sizeof *events,by_time);
 FILE *f=fopen(argv[2],"wb");if(!f)return 2;
 int32_t raw[BLOCK];float left[BLOCK],right[BLOCK],send[BLOCK],delay[BLOCK],interleaved[2*BLOCK];
 for(unsigned pos=0;pos<FRAMES;pos+=BLOCK){
  while(cursor<count&&events[cursor].frame<pos+BLOCK){event e=events[cursor++];
   if(mode==0){unsigned k=round_robin[e.role]++&1u;drum808_trigger(&kit[k],e.role,(float)e.velocity/127);}
   else if(mode==1){if(e.off)bass303_note_off(&bass);else bass303_note_on(&bass,e.role,0,bass.gate,0);}
   else {uint8_t m[3]={(uint8_t)(e.off?0x80:0x90),(uint8_t)e.role,(uint8_t)e.velocity};if(synth_dx7_midi(fm,patch,m,3))return 5;}
  }
  memset(raw,0,sizeof raw);memset(left,0,sizeof left);memset(right,0,sizeof right);memset(send,0,sizeof send);memset(delay,0,sizeof delay);
  if(mode==0){for(int k=0;k<2;k++)drum808_render_st(&kit[k],left,right,send,delay,BLOCK);}
  else {if(mode==1)bass303_render(&bass,raw,BLOCK);else if(synth_dx7_render(fm,raw,BLOCK))return 6;
   for(int i=0;i<BLOCK;i++)left[i]=right[i]=(float)raw[i]/16777216.0f;
  }
  unsigned n=FRAMES-pos<BLOCK?FRAMES-pos:BLOCK;
  for(unsigned i=0;i<n;i++){if(!isfinite(left[i])||!isfinite(right[i]))return 7;interleaved[2*i]=left[i];interleaved[2*i+1]=right[i];}
  if(fwrite(interleaved,sizeof(float),n*2,f)!=n*2)return 8;
 }
 fclose(f);if(fm)synth_dx7_destroy(fm);printf("%s: %u events, %u frames, 44.1 kHz\n",argv[1],count,FRAMES);return 0;
}
