/* SPDX-License-Identifier: GPL-3.0-only */
static void b174_restore(b174_engine *e){e->set=e->saved;b174_tempo(e,e->set.bpm);e->held=0;e->fill_until=0;e->drum_mute=0;}
void b174_command(b174_engine *e,int c,int value){
 switch(c){
 case B174_C_PLAY:if(value&&!e->running){e->step_remaining=0;e->step=0;e->bar=0;e->frame=0;e->write_count=0;e->read_position=0;e->read_q16=0;e->print_ready=0;e->fill_until=0;e->drum_mute=0;e->held=0;e->roll_at=0;e->rng=e->set.seed;}e->running=value!=0;e->manual_audition=0;if(!value){e->held=0;e->bass_target=0;e->roll_at=0;}break;
 case B174_C_BPM:b174_tempo(e,value);break;
 case B174_C_STYLE:e->set.style=(uint8_t)b174_clamp(value,0,2);break;
 case B174_C_FAMILY:e->set.family=(uint8_t)b174_clamp(value,0,6);break;
 case B174_C_COMPLEXITY:e->set.complexity=(uint8_t)b174_clamp(value,0,127);break;
 case B174_C_ROLL:e->set.roll=(uint8_t)b174_clamp(value,0,127);break;
 case B174_C_CRUNCH:e->set.crunch=(uint8_t)b174_clamp(value,0,127);break;
 case B174_C_MIX:e->set.mix=(uint8_t)b174_clamp(value,0,127);break;
 case B174_C_SEED:e->set.seed=(uint32_t)value? (uint32_t)value:1;e->rng=e->set.seed;break;
 case B174_C_BASS:e->set.bass=(uint8_t)(value!=0);if(!value)e->bass_target=0;break;
 case B174_C_KIT:e->set.kit=(uint8_t)b174_clamp(value,0,3);break;
 case B174_C_ROOT:e->set.root=(uint8_t)b174_clamp(value,0,11);break;
 case B174_C_AIR:e->set.air=(uint8_t)b174_clamp(value,0,127);break;
 case B174_C_HELD:e->held=(uint32_t)value&2047u;break;
 case B174_C_RETURN:b174_restore(e);break;
 case B174_C_FILL:e->fill_until=e->frame+(e->step_q16>>16)*(uint32_t)b174_clamp(value,1,16);break;
 case B174_C_SCENE:e->pending_scene=(uint8_t)b174_clamp(value,0,7);e->scene_pending=1;break;
 case B174_C_PANIC:e->held=0;e->running=0;e->bass_target=0;e->bass_env=0;e->bass_gate=0;e->air_env=0;e->ep_env=0;e->pad_left=0;e->pad_right=0;e->space_lp=0;e->event_count=e->event_cursor=0;e->room_lp=0;e->dub_lp=0;for(int v=0;v<B174_VOICES;v++)e->voice[v].env=0;for(unsigned i=0;i<B174_ROOM;i++)e->room[i]=0;for(unsigned i=0;i<B174_DUB;i++)e->dub[i]=0;for(unsigned i=0;i<B174_SPACE;i++)e->space[i]=0;break;
 case B174_C_CLOCK: /* MIDI realtime; interval tracking is applied only when selected by adapter. */
  if(value==0xfa)b174_command(e,B174_C_PLAY,1);
  else if(value==0xfc)b174_command(e,B174_C_PLAY,0);
  else if(value==0xfb)e->running=1;
  else if(value==0xf8){uint32_t delta=e->frame-e->last_clock_frame;e->last_clock_frame=e->frame;e->clock_ticks++;if(e->external_clock&&delta>400&&delta<1200)b174_tempo(e,(int)((B174_RATE*60u)/(24u*delta)));}
  break;
 default:break;
 }
}
void b174_key(b174_engine *e,int black,int index,int down){
 if(index<0||index>=(black?11:16))return;
 if(black){if(down)e->held|=1u<<index;else e->held&=~(1u<<index);e->roll_at=0;return;}
 if(!down)return;
 if(index<7)b174_command(e,B174_C_FAMILY,index);
 else if(index==7)b174_command(e,B174_C_SEED,(int)(e->set.seed+1));
 else if(index>=8&&index<=11)b174_command(e,B174_C_FILL,index==8?4:index==9?8:16);
 else if(index==12){b174_hit(e,B174_KICK,125);b174_hit(e,B174_HAT,90);}
 else if(index==13)e->drum_mute=1;
 else if(index==14)e->drum_mute=0;
 else b174_command(e,B174_C_RETURN,0);
}
