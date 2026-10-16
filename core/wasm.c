/* SPDX-License-Identifier: GPL-3.0-only */
#include "break174.h"
#include <stddef.h>
void *memcpy(void *d,const void *s,size_t n){unsigned char *a=d;const unsigned char *b=s;while(n--)*a++=*b++;return d;}
void *memset(void *d,int c,size_t n){unsigned char *a=d;while(n--)*a++=(unsigned char)c;return d;}
static b174_engine instrument;
static int32_t audio_buffer[512];
void init(int seed){b174_init(&instrument,44100,(uint32_t)seed);}
void command(int c,int v){b174_command(&instrument,c,v);}
void key(int black,int index,int down){b174_key(&instrument,black,index,down);}
void hit(int role,int velocity){b174_hit(&instrument,role,velocity);}
int status(int field){return b174_status(&instrument,field);}
int render(int n){if(n<0)n=0;if(n>256)n=256;b174_render(&instrument,audio_buffer,(uint32_t)n);return (int)(uintptr_t)audio_buffer;}
int state_size(void){return (int)sizeof instrument;}
