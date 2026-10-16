/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef B174_INTERNAL_H
#define B174_INTERNAL_H
static int b174_clamp(int x,int lo,int hi){return x<lo?lo:x>hi?hi:x;}
static uint32_t b174_random(b174_engine *e){uint32_t x=e->rng;x^=x<<13;x^=x>>17;x^=x<<5;return e->rng=x?x:1;}
#include "sine_rom.h"
static int32_t b174_sine(uint32_t ph){
 unsigned at=ph>>22,frac=(ph>>6)&65535u;
 int32_t a=b174_sine_rom[at],d=b174_sine_rom[at+1]-a;
 return a+(d*(int32_t)frac)/65536;
}
/* Keep saturation separate: the pi32 compiler otherwise folds gain+clip into a
 * comparison against an immediate in the protected ROM address range. */
static __attribute__((noinline)) int32_t b174_clip(int32_t x){return b174_clamp(x,-32767,32767);}
static void b174_tempo(b174_engine *e,int bpm){e->set.bpm=(uint16_t)b174_clamp(bpm,140,190);e->step_q16=(((B174_RATE*15u)<<12)/e->set.bpm)<<4;}
static void b174_step(b174_engine *e);
static int32_t b174_voice_sample(b174_voice *v);
#endif
