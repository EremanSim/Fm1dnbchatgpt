/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef B174_SILENCE_H
#define B174_SILENCE_H
#include <stdint.h>
/* Four fully silent rendered DMA halves cover the queued and currently playing half. */
static uint32_t b174_silence_blocks(uint32_t previous,const int32_t *out,uint32_t frames,int running){if(running)return 0;for(uint32_t i=0;i<frames*2u;i++)if(out[i])return 0;return previous<4?previous+1:4;}
#endif
