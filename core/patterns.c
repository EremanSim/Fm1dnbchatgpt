/* SPDX-License-Identifier: GPL-3.0-only */
const char *b174_style_name(int n){static const char *const names[]={"DEEP","JUNGLE","ATMOS"};return names[b174_clamp(n,0,2)];}
const char *b174_family_name(int n){static const char *const names[]={"SOUL TWO STEP","AMEN FLOW","BRUSHED THINK","FUNKY GHOST","APACHE JAZZ","DEEP ROLLER","HALFTIME"};return names[b174_clamp(n,0,6)];}
static void b174_step(b174_engine *e){
 unsigned s=e->step&15u,bar=e->bar&3u,f=e->set.family,unit=e->step_q16>>16;
 uint32_t rnd=b174_random(e);unsigned busy=e->set.complexity;
 int fill=e->fill_until&&e->frame<e->fill_until;
 e->event_count=e->event_cursor=0;
 if(s==0)b174_harmony(e);
 /* Velocity and late offbeats establish a groove instead of an even noise grid. */
 if(f==6){if(s==0||s==10)b174_schedule(e,B174_KICK,106,0);if(s==8)b174_schedule(e,B174_SNARE,101,70);}
 else{
  int kick=s==0||s==10;
  if(f==1)kick=s==0||s==6||(bar&1u?s==11:s==10);
  if(f==4)kick=s==0||s==7||s==10;
  if(f==3)kick=s==0||s==9||((bar&1u)&&s==14);
  if(kick)b174_schedule(e,B174_KICK,(s==0?110:88)-(e->set.style==2?6:0),0);
  if(s==4||s==12)b174_schedule(e,B174_SNARE,(s==4?105:98)+(e->set.style==1?5:0)-(e->set.style==2?8:0),55+(bar&1u)*28u);
  if((f==1||f==2||f==3||e->set.style==1)&&(s==3||s==7||s==11||s==15)){
   unsigned velocity=s==7?34:24+(rnd%15u);
   b174_schedule(e,B174_GHOST,velocity,90+(s==15?unit/2:unit/10));
  }
  if(f==4&&(s==5||s==13))b174_schedule(e,B174_TOM,48,unit/3);
  if(f==2&&(s==2||s==10))b174_schedule(e,B174_PERC,36,unit/2);
 }
 if(!(s&1u))b174_schedule(e,B174_HAT,s&2u?46:64,s&2u?100:0);
 else if((rnd%128u)<busy||f==5)b174_schedule(e,B174_HAT,24+(rnd%14u),unit/8);
 if((s==2||s==6||s==10||s==14)&&(f==1||e->set.style==2))b174_schedule(e,B174_RIDE,s==2?46:33,130);
 if(s==14&&f!=6&&bar&1u)b174_schedule(e,B174_OPEN,40,unit/6);
 if(fill||((e->held&(1u<<3))&&s&1u)){
  b174_schedule(e,B174_GHOST,40+s*2,unit/2);
  if(fill&&s&1u)b174_schedule(e,B174_SNARE,48+s,unit*3/4);
 }
 if(e->set.roll&&s>=14&&((rnd>>16)%128)<e->set.roll)b174_schedule(e,B174_SNARE,62,unit/2);
 /* Four-bar conversation, with actual gaps. Harmony changes every two bars. */
 if(s==0)b174_bass_note(e,0,3);
 if(s==6)b174_bass_note(e,bar&1u?7:0,2);
 if(s==10)b174_bass_note(e,0,3);
 if(s==15&&bar==3)b174_bass_note(e,10,1);
 if((s==2&&!(e->bar&1u))||(s==11&&(e->bar&1u))){
  static const uint8_t keynotes[4]={74,72,79,77};
  e->ep_inc=b174_midi_inc[keynotes[(e->bar/2)&3]+e->set.root];e->ep_env=22500;
 }
 if(s==0)e->print_gate=1;
}
