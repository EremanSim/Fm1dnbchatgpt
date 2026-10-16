/* SPDX-License-Identifier: GPL-3.0-only
 * Original procedural PCM synthesized by tools/design_drums.py, rendered from ROM.
 * Keeping the detailed resonators out of the ISR also leaves room for harmony. */
#include "drum_rom.h"
void b174_hit(b174_engine *e,int role,int velocity){
 if(role<0||role>=B174_VOICES)return;
 if(!e->running)e->manual_audition=1;
 b174_voice *v=&e->voice[role];
 v->role=(uint8_t)role;v->velocity=(uint16_t)b174_clamp(velocity,1,127);v->env=32700;v->position=0;v->low=0;
 /* Subtle tuning variation, not randomized timing of the main backbeat. */
 uint32_t h=e->set.seed+e->hit_count++*1031u+(unsigned)role*7919u;
 v->advance=32768u+(uint32_t)e->set.kit*600u+(h%401u)-200u;
 if(e->set.kit==2)v->advance=30500u+h%300u;
 if(role==B174_HAT)e->voice[B174_OPEN].env/=4;
}
static int32_t b174_voice_sample(b174_voice *v){
 unsigned at=v->position>>16,role=v->role;
 if(!v->env||at+1>=b174_rom_len[role]){v->env=0;return 0;}
 unsigned frac=v->position&65535u;
 const int16_t *p=b174_rom[role];int32_t a=p[at],delta=p[at+1]-a;
 /* Split the product to keep extreme adjacent PCM differences within int32. */
 int32_t sample=a+(delta*(int32_t)(frac>>1))/32768;
 v->position+=v->advance;
 sample=(sample*(int32_t)v->velocity)/127;
 if(role==B174_HAT||role==B174_OPEN||role==B174_RIDE)sample=sample*3/4;
 sample=sample*v->env/32768;
 v->last=sample;return sample;
}
/* Stable, bounded per-step event queue. Offbeats breathe; clock remains exact. */
static void b174_schedule(b174_engine *e,unsigned role,unsigned velocity,unsigned delay){
 if(e->event_count>=12)return;
 unsigned n=e->event_count++;
 while(n&&e->event[n-1].delay>delay){e->event[n]=e->event[n-1];n--;}
 e->event[n].delay=(uint16_t)delay;e->event[n].role=(uint8_t)role;e->event[n].velocity=(uint8_t)b174_clamp((int)velocity,1,127);
}
static void b174_events(b174_engine *e){
 while(e->event_cursor<e->event_count&&e->event[e->event_cursor].delay<=e->step_elapsed){
  b174_event *p=&e->event[e->event_cursor++];b174_hit(e,p->role,p->velocity);
 }
}
