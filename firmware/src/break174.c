/* SPDX-License-Identifier: GPL-3.0-only
 * BREAK//174 FM-1 adapter. Platform: Felucca/SLOOP/Dinghy; see docs/upstream. */
#define APP_NAME "BREAK174"
#include "platform.c"
#include "../../core/break174.c"
#include "../../core/input.h"
#include "../../core/silence.h"

/* Large audio state lives in the bounded pool, not the 96 KiB globals area. */
static b174_engine engine __attribute__((section(".pool")));
typedef struct {int32_t kind,value;} app_message;
static app_message commands[64];
static volatile uint32_t command_write,command_read,queue_panic;
static volatile uint32_t silent_blocks;
static void enqueue(int kind,int value){
 uint32_t w=command_write;
 if(w-command_read>=64u){queue_panic=1;return;}
 commands[w&63u].kind=kind;commands[w&63u].value=value;
 RING_PUBLISH();command_write=w+1u;
}
static void app_audio(int32_t *out,uint32_t n){
 if(queue_panic){b174_command(&engine,B174_C_PANIC,0);command_read=command_write;queue_panic=0;}
 while(command_read!=command_write){app_message m=commands[command_read&63u];command_read++;
  if(m.kind==100)b174_key(&engine,1,m.value&15,(m.value>>4)&1);
  else if(m.kind==101)b174_key(&engine,0,m.value,1);
  else if(m.kind>=200&&m.kind<208)b174_hit(&engine,m.kind-200,m.value);
  else b174_command(&engine,m.kind,m.value);
 }
 b174_render(&engine,out,n);
 silent_blocks=b174_silence_blocks(silent_blocks,out,n,engine.running);
}
static int is_black(uint32_t note){return (0x54Au>>(note%12u))&1u;}
static void number(char *s,uint32_t n){char t[12];unsigned i=0,j=0;do{t[i++]=(char)('0'+n%10u);n/=10u;}while(n);while(i)s[j++]=t[--i];s[j]=0;}
static void label_value(int y,const char *name,uint32_t v){char num[12];draw_text_box(8,y,140,&FONT_S,name,RGB(130,140,150),0);number(num,v);draw_text_box(150,y,80,&FONT_S,num,RGB(0,230,170),2);}
static void screen(int mode,int pending){
 char line[20];
 draw_text_box(0,8,240,&FONT_L,"BREAK//174",RGB(0,230,170),1);
 draw_text_box(0,44,240,&FONT_S,b174_style_name(engine.set.style),RGB(255,170,70),1);
 draw_text_box(0,66,240,&FONT_S,mode?"DRUM KEYS":engine.set.air?"LIVE / ATMOS ON":"LIVE / ATMOS OFF",C_WHITE,1);
 draw_text_box(0,90,240,&FONT_S,b174_family_name(engine.set.family),C_WHITE,1);
 label_value(116,"BPM",engine.set.bpm);label_value(138,"COMPLEX",engine.set.complexity);
 label_value(158,"ROLL",engine.set.roll);label_value(178,"CRUNCH",engine.set.crunch);
 label_value(198,"PRINT MIX",engine.set.mix);
 number(line,engine.bar+1u);
 draw_text_box(0,221,240,&FONT_S,pending?"SAVE AFTER STOP":engine.running?"PLAY  /  RETURN W16":"STOP  /  PLAY TO START",RGB(130,140,150),1);
}
static void app_main(void){
 b174_settings settings;uint32_t prev=0,last_ui=0,last_holds=0,blocked=0;int mode=0,save_pending=0;
 b174_holds holds={0};
 b174_init(&engine,44100,174);
 if(!settings_load(&settings,sizeof settings)){
  /* Restore through validated commands, never trust flash bytes as engine state. */
  b174_command(&engine,B174_C_BPM,settings.bpm);b174_command(&engine,B174_C_STYLE,settings.style);
  b174_command(&engine,B174_C_FAMILY,settings.family);b174_command(&engine,B174_C_SEED,(int)settings.seed);
  b174_command(&engine,B174_C_COMPLEXITY,settings.complexity);b174_command(&engine,B174_C_ROLL,settings.roll);
  b174_command(&engine,B174_C_CRUNCH,settings.crunch);b174_command(&engine,B174_C_MIX,settings.mix);
  b174_command(&engine,B174_C_BASS,settings.bass);b174_command(&engine,B174_C_KIT,settings.kit);
  b174_command(&engine,B174_C_ROOT,settings.root);b174_command(&engine,B174_C_AIR,settings.air);engine.set.scene=(uint8_t)b174_clamp(settings.scene,0,7);engine.saved=engine.set;
 }
 lcd_fill(0,0,240,240,C_BLACK);screen(mode,save_pending);
 for(;;){
  uint32_t keys,down,buttons,released,w=0,b=0;uint8_t st,d1,d2;int delta;
  platform_poll();keys=fm1_in.notes;down=keys&~prev;prev=keys;
  blocked&=keys;holds.physical=0;
  for(unsigned k=0;k<27;k++){
   int black=is_black(53u+k);unsigned ix=black?b++:w++;
   if(mode){if((down>>k)&1u)enqueue(200+(int)(ix%8u),100);}
   else if(black){if(((keys&~blocked)>>k)&1u)holds.physical|=(uint16_t)(1u<<ix);}
   else if((down>>k)&1u){enqueue(101,(int)ix);if(ix==15){memset(&holds,0,sizeof holds);blocked=keys;}}
   fm1_led_key(KEY_LED(k),mode?((keys>>k)&1u):black?((engine.held>>ix)&1u):(ix==((engine.step-1u)&15u)&&engine.running));
  }
  buttons=fm1_input_edges(&released);
  if(buttons&(1u<<BTN_PLAY))enqueue(B174_C_PLAY,!engine.running);
  if(buttons&(1u<<BTN_HOME))mode=0;
  if(buttons&(1u<<BTN_SEL)){mode=!mode;holds.physical=0;blocked=keys;}
  if(buttons&(1u<<BTN_SAVE))save_pending=1;
  if(buttons&(1u<<BTN_OCT_UP))enqueue(B174_C_SEED,(int)(engine.set.seed+1u));
  if(buttons&(1u<<BTN_OCT_DOWN)){enqueue(B174_C_RETURN,0);memset(&holds,0,sizeof holds);blocked=keys;}
  if(buttons&(1u<<BTN_SEQ))enqueue(B174_C_FILL,16);
  if(buttons&(1u<<BTN_ARP))enqueue(B174_C_BASS,!engine.set.bass);
  if(buttons&(1u<<BTN_FX))enqueue(B174_C_AIR,engine.set.air?0:86);
  if(buttons&(1u<<BTN_EDIT))enqueue(B174_C_KIT,(engine.set.kit+1)%4);
  delta=fm1_enc_take(ENC_SELECT);if(delta)enqueue(B174_C_BPM,engine.set.bpm+delta);
  delta=fm1_enc_take(ENC_ALGORITHM);if(delta)enqueue(B174_C_STYLE,b174_clamp((int)engine.set.style+delta,0,2));
  delta=fm1_enc_take(ENC_PRESETS);if(delta)enqueue(B174_C_SCENE,b174_clamp((int)engine.set.scene+delta,0,7));
  {int cmd[4]={B174_C_COMPLEXITY,B174_C_ROLL,B174_C_CRUNCH,B174_C_MIX};unsigned val[4]={engine.set.complexity,engine.set.roll,engine.set.crunch,engine.set.mix};for(int i=0;i<4;i++){delta=fm1_enc_take(ENC_KNOB1+i);if(delta)enqueue(cmd[i],(int)val[i]+delta*2);}}
  while(midi_get(&st,&d1,&d2)){
   if(st>=0xf8){enqueue(B174_C_CLOCK,st);continue;}
   if((st&0xf0)==0x90&&d2){if((st&15)==9){int role=b174_gm_role(d1);if(role>=0)enqueue(200+role,d2);}else if(d1>=36&&d1<52){enqueue(101,d1-36);if(d1==51){memset(&holds,0,sizeof holds);blocked=keys;}}else b174_holds_midi(&holds,st&15,d1,1);}
   else if((st&0xf0)==0x80||((st&0xf0)==0x90&&!d2))b174_holds_midi(&holds,st&15,d1,0);
   else if((st&0xf0)==0xb0){if(d1==123||d1==120){memset(&holds,0,sizeof holds);blocked=keys;enqueue(B174_C_PANIC,0);}else if(d1>=20&&d1<24)enqueue(B174_C_COMPLEXITY+d1-20,d2);}
  }
  {uint32_t mask=b174_holds_mask(&holds);if(mask!=last_holds){enqueue(B174_C_HELD,(int)mask);last_holds=mask;}}
  if(save_pending&&!engine.running&&silent_blocks>=4&&command_read==command_write){settings=engine.set;if(!settings_save(&settings,sizeof settings)){engine.saved=settings;save_pending=0;}}
  if(fm1_ms-last_ui>=100u){screen(mode,save_pending);last_ui=fm1_ms;}
  fm1_led_key(BTN_PLAY,engine.running);fm1_led_key(BTN_SAVE,save_pending);
 }
}
