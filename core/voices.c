/* SPDX-License-Identifier: GPL-3.0-only
 * Original procedural percussion. No drum ROMs or recorded breaks. */
void b174_hit(b174_engine *e,int role,int velocity){
 b174_voice *v;uint32_t hz;
 if(role<0||role>=B174_VOICES)return;
 if(!e->running)e->manual_audition=1;
 velocity=b174_clamp(velocity,1,127);v=&e->voice[role];
 v->role=(uint8_t)role;v->velocity=(uint16_t)velocity;v->env=32700;
 v->phase=0x80000000u;v->phase2=0;v->low=0;v->band=0;
 v->noise=e->set.seed+(uint32_t)role*7919u+e->hit_count++*1031u+1u;
 v->sweep=0;
 switch(role){
 case B174_KICK:hz=65u+e->set.kit*5u;v->sweep=18000000u;v->decay=32753;break;
 case B174_SNARE:hz=178u+e->set.kit*23u;v->decay=32747;break;
 case B174_GHOST:hz=195u+e->set.kit*16u;v->decay=32725;break;
 case B174_HAT:hz=6500;v->decay=32630;e->voice[B174_OPEN].env/=4;break;
 case B174_OPEN:hz=5300;v->decay=32748;break;
 case B174_RIDE:hz=3100;v->decay=32759;break;
 case B174_TOM:hz=130;v->decay=32752;v->sweep=2200000;break;
 default:hz=850;v->decay=32710;break;
 }
 v->inc=(hz*97391u); /* 2^32 / 44100, integer approximation */
}
static int32_t b174_voice_sample(b174_voice *v){
 int32_t n,tone,s=0,hi;uint32_t r;
 if(v->env<8){v->env=0;return 0;}
 r=v->noise;r^=r<<13;r^=r>>17;r^=r<<5;v->noise=r;
 n=(int32_t)(r>>16)-32768;
 v->phase+=v->inc+v->sweep;v->phase2+=v->inc*3u/2u;
 v->sweep-=v->sweep>>8;
 tone=b174_sine(v->phase);v->low+=(n-v->low)>>3;hi=n-v->low;
 switch(v->role){
 case B174_KICK:s=(tone*3+v->low)/4;break;
 case B174_SNARE:s=(tone+(hi*3)/2+b174_sine(v->phase2)/2)/3;break;
 case B174_GHOST:s=(tone+v->low*2)/4;break;
 case B174_HAT:case B174_OPEN:s=hi/3;break;
 case B174_RIDE:s=(hi/2+tone/4+b174_sine(v->phase2)/4)/2;break;
 case B174_TOM:s=(tone*3+v->low)/4;break;
 default:s=(tone+hi)/3;break;
 }
 v->env=(v->env*v->decay)>>15;
 s=((s*v->env)>>15)*v->velocity/190;
 v->last=s;return s;
}
