/* SPDX-License-Identifier: GPL-3.0-only */
#include "break174.h"
#include "internal.h"
#include "voices.c"
#include "music.c"
#include "patterns.c"
#include "performance.c"

void b174_init(b174_engine *e,uint32_t rate,uint32_t seed){
 uint8_t *p=(uint8_t *)e;for(unsigned i=0;i<sizeof *e;i++)p[i]=0;
 (void)rate; /* Native engine is always 44100; adapters must resample. */
 e->set.seed=seed?seed:1;e->rng=e->set.seed;e->set.bass=1;e->set.mix=0;e->set.complexity=40;e->set.style=2;e->set.family=1;e->set.root=5;e->set.air=86;
 b174_tempo(e,166);e->saved=e->set;e->initialized=1;
}
static void b174_scene_commit(b174_engine *e){
 unsigned s=e->pending_scene;e->set.scene=(uint8_t)s;e->set.family=(uint8_t)(s%7);e->set.style=(uint8_t)(s<3?0:s<6?1:2);
 e->set.kit=(uint8_t)(s%4);e->set.complexity=(uint8_t)(24+s*6);e->set.crunch=(uint8_t)(e->set.style==1?12:0);e->set.air=(uint8_t)(s<3?60:s<6?35:96);
 e->saved=e->set;e->scene_pending=0;
}
static int32_t b174_print_sample(b174_engine *e){
 uint32_t n=e->pcm_length[e->ready_bank],pos;int32_t out;
 if(!e->print_ready||n<2)return 0;
 if(e->held&(1u<<4))pos=(n-1u)-(e->read_position%n);else pos=e->read_position%n;
 out=e->pcm[e->ready_bank][pos];
 e->read_q16+=((e->held&(1u<<5))?16384:32768)>>((e->held&(1u<<7))?1:0);
 if(e->read_q16>=65536){e->read_q16-=65536;e->read_position++;}
 return out;
}
void b174_render(b174_engine *e,int32_t *out,uint32_t frames){
 for(unsigned i=0;i<frames;i++){
  int32_t dry=0,room,drums,bass=0,left,right,delayed,echo,air_l,air_r;
  if(!e->initialized){out[2*i]=out[2*i+1]=0;continue;}
  if(e->running){
   if(e->step_remaining<=65536u){
    e->step_remaining+=e->step_q16;e->step_elapsed=0;
    if((e->step&15u)==0){
     if(e->step){e->bar++;if(e->write_count>1){e->pcm_length[e->write_bank]=e->write_count;e->ready_bank=e->write_bank;e->print_ready=1;e->write_bank^=1u;}e->write_count=0;e->read_position=0;}
     if(e->scene_pending)b174_scene_commit(e);
    }
    b174_step(e);e->step++;
   }
   e->step_remaining-=65536u;b174_events(e);e->step_elapsed++;
   if(e->held&7u){unsigned div=e->held&4u?4:e->held&2u?2:1;uint32_t interval=(e->step_q16>>16)/div;if(!e->roll_at||e->frame>=e->roll_at){b174_hit(e,B174_SNARE,87);e->roll_at=e->frame+interval;}}
   e->frame++;
  }
  for(int v=0;v<B174_VOICES;v++)dry+=b174_voice_sample(&e->voice[v]);
  room=e->room[(e->room_pos+1831u)&(B174_ROOM-1u)];
  delayed=e->room[(e->room_pos+701u)&(B174_ROOM-1u)];
  e->room_lp+=(room-e->room_lp)>>2;
  e->room[e->room_pos]=(int16_t)b174_clip(dry/3+e->room_lp/3);
  e->room_pos=(e->room_pos+1u)&(B174_ROOM-1u);
  dry=b174_clip(dry*3/5+room/12+delayed/16);
  e->wet_lp+=(dry-e->wet_lp)>>1;e->decimate^=1u;
  if(e->running&&e->decimate&&e->write_count<B174_PCM)e->pcm[e->write_bank][e->write_count++]=(int16_t)e->wet_lp;
  drums=dry;
  if(e->print_ready&&e->running){int32_t print=b174_print_sample(e);unsigned mix=e->held&((1u<<4)|(1u<<5)|(1u<<7))?127:e->set.mix;drums=(dry*(127-(int)mix)+print*(int)mix)/127;}
  if(e->held&(1u<<6))drums=(e->step_elapsed<600)?drums:0;
  if(e->set.crunch){unsigned shift=1u+e->set.crunch/24u;int32_t quant=(drums/(1<<shift))*(1<<shift);drums=drums+(quant-drums)*(int)e->set.crunch/127;drums=b174_clip(drums*(128+(int)e->set.crunch)/128);}
  if(e->held&(1u<<9))drums-=e->wet_lp;
  /* Dotted eighth, with a filtered feedback tail after the throw is released. */
  {uint32_t time=(e->step_q16>>16)*3u;
   echo=e->dub[(e->dub_pos-time)&(B174_DUB-1u)];
   e->dub_lp+=(echo-e->dub_lp)/3;
   if(e->dub_lp<3&&e->dub_lp>-3)e->dub_lp=0;
   e->dub[e->dub_pos]=(int16_t)b174_clip(((e->held&(1u<<8))?drums/2:0)+e->dub_lp*3/5);
   e->dub_pos=(e->dub_pos+1u)&(B174_DUB-1u);
   drums+=echo/2;
  }
  if(e->drum_mute||e->held&(1u<<10))drums=0;
  if(e->bass_gate)e->bass_gate--;else e->bass_target=0;
  if(!e->running)e->bass_target=0;
  {int32_t diff=e->bass_target-e->bass_env;int32_t smooth=diff>0?512:1536;e->bass_env+=diff>0?(diff+smooth-1)/smooth:(diff-smooth+1)/smooth;}
  if(e->bass_env>0){
   int32_t pitch=(int32_t)e->bass_goal-(int32_t)e->bass_inc;e->bass_inc=(uint32_t)((int32_t)e->bass_inc+pitch/800);
   e->bass_phase+=e->bass_inc;
   bass=(b174_sine(e->bass_phase)+b174_sine(e->bass_phase*2u)/7)*e->bass_env/32768;
   if(e->voice[B174_KICK].env&&(e->voice[B174_KICK].position>>16)<1700u)bass=bass*4/5;
  }
  b174_music(e,&air_l,&air_r);
  left=b174_clip(drums+bass+air_l+((e->drum_mute||e->held&(1u<<10))?0:room/28));
  right=b174_clip(drums+bass+air_r+((e->drum_mute||e->held&(1u<<10))?0:delayed/28));
  if(!e->running&&!e->manual_audition){left=left*e->master_env/32768;right=right*e->master_env/32768;if(e->master_env>0)e->master_env-=32;else e->master_env=0;}
  else if(e->master_env<32768)e->master_env+=64;
  out[2*i]=left;out[2*i+1]=right;
  {uint32_t peak=(uint32_t)(left<0?-left:left);e->peak=peak>e->peak?peak:e->peak?e->peak-(e->peak/4096+1):0;}
 }
}
int b174_status(const b174_engine *e,int f){
 switch(f){case B174_S_BPM:return e->set.bpm;case B174_S_RUNNING:return e->running;case B174_S_FRAME:return (int)e->frame;case B174_S_STEP:return e->step?(int)((e->step-1u)&15u):0;case B174_S_BAR:return (int)e->bar;case B174_S_STYLE:return e->set.style;case B174_S_FAMILY:return e->set.family;case B174_S_PRINT:return e->print_ready;case B174_S_HELD:return (int)e->held;case B174_S_SCENE:return e->set.scene;case B174_S_PEAK:return (int)e->peak;case B174_S_COMPLEXITY:return e->set.complexity;case B174_S_ROLL:return e->set.roll;case B174_S_CRUNCH:return e->set.crunch;case B174_S_MIX:return e->set.mix;case B174_S_BASS:return e->set.bass;case B174_S_SEED:return (int)e->set.seed;case B174_S_KIT:return e->set.kit;case B174_S_ROOT:return e->set.root;case B174_S_FILL:return e->fill_until>e->frame;case B174_S_CLOCK:return e->external_clock;case B174_S_AIR:return e->set.air;default:return 0;}
}
