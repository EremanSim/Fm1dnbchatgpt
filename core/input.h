/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef B174_INPUT_H
#define B174_INPUT_H
#include <stdint.h>
/* Each source owns its hold state. MIDI drum channel never owns FX. */
typedef struct {uint16_t physical,midi[16];} b174_holds;
static uint32_t b174_holds_mask(const b174_holds *h){uint32_t m=h->physical;for(unsigned c=0;c<16;c++)m|=h->midi[c];return m&2047u;}
static void b174_holds_midi(b174_holds *h,unsigned channel,unsigned note,int down){if(channel>=16||channel==9||note<60||note>70)return;uint16_t mask=(uint16_t)(1u<<(note-60));if(down)h->midi[channel]|=mask;else h->midi[channel]&=(uint16_t)~mask;}
static int b174_gm_role(unsigned n){switch(n){case 35:case 36:return 0;case 38:case 40:return 1;case 39:return 2;case 42:case 44:return 3;case 46:return 4;case 49:case 51:case 57:return 5;case 41:case 43:case 45:case 47:case 48:case 50:return 6;case 37:case 54:case 56:return 7;default:return -1;}}
#endif
