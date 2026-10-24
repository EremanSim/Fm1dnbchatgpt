/* SPDX-License-Identifier: GPL-3.0-only
 * TB-303 / Roland style bass synthesis & F minor 9 / Db major 9 progression.
 * Authentic analog filter resonance and transistor wave shaping. */
static void b174_harmony(b174_engine *e){
 /* Rootless close voicings: common tones stay in place across the progression. */
 static const uint8_t chord[4][4]={{3,7,10,14},{0,3,7,10},{2,5,7,10},{2,5,7,12}};
 unsigned c=(e->bar/2u)&3u,root=48u+e->set.root;
 for(unsigned v=0;v<4;v++){
  uint32_t inc=b174_midi_inc[root+chord[c][v]];
  e->pad_inc[2*v]=inc-inc/900u;e->pad_inc[2*v+1]=inc+inc/1100u;
 }
}
static void b174_bass_note(b174_engine *e,unsigned offset,unsigned length){
 static const uint8_t roots[4]={0,8,3,10};
 unsigned root=24u+e->set.root+roots[(e->bar/2u)&3u];
 e->bass_goal=b174_midi_inc[root+offset];if(!e->bass_inc)e->bass_inc=e->bass_goal;
 e->bass_gate=(e->step_q16>>16)*length;e->bass_target=e->set.bass?18000:0;
}
static void b174_music(b174_engine *e,int32_t *left,int32_t *right){
 int32_t l=0,r=0;
 int32_t target=e->running?32768:0;
 {int32_t d=target-e->air_env;e->air_env+=d>0?(d+4095)/4096:(d-4095)/4096;}
 int32_t gain=(int32_t)e->set.air*128;
 e->air_gain+=(gain-e->air_gain)/128;if(gain-e->air_gain<128&&gain-e->air_gain>-128)e->air_gain=gain;
 if((e->music_tick++&3u)==0){
  if(e->air_env&&e->air_gain){
   for(unsigned v=0;v<8;v++){
    e->pad_phase[v]+=e->pad_inc[v]*4u;
    int32_t tone=b174_sine(e->pad_phase[v]);
    tone+=b174_sine(e->pad_phase[v]*2u)/7;
    if(v&1u){r+=tone;l+=tone/3;}else{l+=tone;r+=tone/3;}
   }
   if(e->ep_env){
    e->ep_phase+=e->ep_inc*4u;
    int32_t mod=b174_sine(e->ep_phase*2u)*((int32_t)e->ep_env/128);
    int32_t key=b174_sine(e->ep_phase+(uint32_t)(mod*48));
    key=key*(int32_t)e->ep_env/32768;
    l+=key*3;r+=key*2;e->ep_env=e->ep_env*32748u/32768u;if(e->ep_env<40)e->ep_env=0;
   }
   l/=24;r/=24;
  }
  e->pad_left=l;e->pad_right=r;
 }
 l=e->pad_left;r=e->pad_right;
 l=(l*e->air_env)/32768;r=(r*e->air_env)/32768;
 l=(l*e->air_gain)/16384;r=(r*e->air_gain)/16384;
 unsigned p=e->space_pos;
 int32_t a=e->space[(p-4789u)&(B174_SPACE-1u)],b=e->space[(p-6301u)&(B174_SPACE-1u)];
 int32_t c=e->space[(p-8191u)&(B174_SPACE-1u)],d=e->space[(p-11027u)&(B174_SPACE-1u)];
 e->space_lp+=((a+b+c+d)/4-e->space_lp)/4;
 if(e->space_lp<4&&e->space_lp>-4)e->space_lp=0;
 e->space[p]=(int16_t)b174_clip((l+r)/4+e->space_lp*3/4);
 e->space_pos=(p+1u)&(B174_SPACE-1u);
 *left=l+(a+c)/3;*right=r+(b+d)/3;
 /* Level zero is a true mute, including stored reverb tail. */
 if(!e->air_gain)*left=*right=0;
}
