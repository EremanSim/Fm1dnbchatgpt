/* SPDX-License-Identifier: GPL-3.0-only */
#include "break174.h"
#include <stdio.h>
#include <stdlib.h>
static b174_engine e;
static void u16(FILE *f,unsigned v){fputc(v&255,f);fputc((v>>8)&255,f);}
static void u32(FILE *f,unsigned v){u16(f,v);u16(f,v>>16);}
int main(int argc,char **argv){
 unsigned seconds=argc>2?(unsigned)atoi(argv[2]):20,frames=B174_RATE*seconds;int family=argc>3?atoi(argv[3]):1;int gestures=argc>6?atoi(argv[6]):0;
 if(argc<2||seconds<1||seconds>180){fprintf(stderr,"usage: render output.wav [seconds 1..180] [family 0..6] [style 0..2] [air 0..127] [gestures 0/1]\n");return 2;}
 FILE *f=fopen(argv[1],"wb");if(!f)return 1;
 fwrite("RIFF",1,4,f);u32(f,36+frames*4);fwrite("WAVEfmt ",1,8,f);u32(f,16);u16(f,1);u16(f,2);u32(f,B174_RATE);u32(f,B174_RATE*4);u16(f,4);u16(f,16);fwrite("data",1,4,f);u32(f,frames*4);
 b174_init(&e,B174_RATE,174);b174_command(&e,B174_C_FAMILY,family);if(argc>4)b174_command(&e,B174_C_STYLE,atoi(argv[4]));if(argc>5)b174_command(&e,B174_C_AIR,atoi(argv[5]));b174_command(&e,B174_C_PLAY,1);
 for(unsigned n=0;n<frames;){int32_t out[512];unsigned count=frames-n>256?256:frames-n;
  if(gestures&&n>=B174_RATE*5&&n<B174_RATE*5+256)b174_key(&e,1,1,1);
  if(gestures&&n>=B174_RATE*6&&n<B174_RATE*6+256)b174_key(&e,1,1,0);
  if(gestures&&n>=B174_RATE*9&&n<B174_RATE*9+256)b174_key(&e,1,4,1);
  if(gestures&&n>=B174_RATE*10&&n<B174_RATE*10+256)b174_key(&e,1,4,0);
  b174_render(&e,out,count);for(unsigned i=0;i<count*2;i++)u16(f,(unsigned)(uint16_t)(int16_t)out[i]);n+=count;
 }
 fclose(f);printf("rendered %u seconds to %s\n",seconds,argv[1]);return 0;
}
