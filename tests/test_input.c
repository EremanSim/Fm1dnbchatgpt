#include "input.h"
#include "silence.h"
#include <assert.h>
#include <stdio.h>
int main(void){assert(b174_gm_role(36)==0);assert(b174_gm_role(38)==1);assert(b174_gm_role(46)==4);assert(b174_gm_role(1)==-1);b174_holds h={0};h.physical=1;b174_holds_midi(&h,0,60,1);b174_holds_midi(&h,0,60,0);assert(b174_holds_mask(&h)==1);h.physical=0;b174_holds_midi(&h,1,60,1);b174_holds_midi(&h,9,60,0);assert(b174_holds_mask(&h)==1);b174_holds_midi(&h,0,60,0);assert(b174_holds_mask(&h)==1);b174_holds_midi(&h,1,60,0);assert(!b174_holds_mask(&h));int32_t out[512]={0};out[511]=1;assert(!b174_silence_blocks(3,out,256,0));out[511]=0;uint32_t count=0;for(int i=0;i<4;i++)count=b174_silence_blocks(count,out,256,0);assert(count==4);assert(!b174_silence_blocks(count,out,256,1));puts("input: ownership and quiet DMA grace checks passed");}
