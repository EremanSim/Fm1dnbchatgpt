/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef BREAK174_H
#define BREAK174_H
#include <stdint.h>
#define B174_RATE 44100u
#define B174_PCM 40960u
#define B174_ROOM 4096u
#define B174_DUB 32768u
#define B174_SPACE 16384u
enum {B174_C_PLAY=1,B174_C_BPM,B174_C_STYLE,B174_C_FAMILY,B174_C_COMPLEXITY,B174_C_ROLL,B174_C_CRUNCH,B174_C_MIX,B174_C_SEED,B174_C_BASS,B174_C_RETURN,B174_C_FILL,B174_C_SCENE,B174_C_KIT,B174_C_ROOT,B174_C_PANIC,B174_C_CLOCK,B174_C_HELD,B174_C_AIR};
enum {B174_S_BPM=1,B174_S_RUNNING,B174_S_FRAME,B174_S_STEP,B174_S_BAR,B174_S_STYLE,B174_S_FAMILY,B174_S_PRINT,B174_S_HELD,B174_S_SCENE,B174_S_PEAK,B174_S_COMPLEXITY,B174_S_ROLL,B174_S_CRUNCH,B174_S_MIX,B174_S_BASS,B174_S_SEED,B174_S_KIT,B174_S_ROOT,B174_S_FILL,B174_S_CLOCK,B174_S_AIR};
enum {B174_KICK,B174_SNARE,B174_GHOST,B174_HAT,B174_OPEN,B174_RIDE,B174_TOM,B174_PERC,B174_VOICES};
typedef struct {uint32_t phase,phase2,inc,sweep,noise,position,advance;int32_t env,decay,low,band,last;uint16_t velocity;uint8_t role;} b174_voice;
typedef struct {uint32_t seed;uint16_t bpm;uint8_t style,family,complexity,roll,crunch,mix,bass,kit,root,scene,air;} b174_settings;
typedef struct {uint16_t delay;uint8_t role,velocity;} b174_event;
typedef struct {
 b174_settings set,saved;
 b174_voice voice[B174_VOICES];
 uint32_t frame,rng,step_q16,step_remaining,step_elapsed,roll_at,roll_interval,bass_phase,bass_inc;
 uint32_t step,bar,held,fill_until,peak,hit_count;
 int32_t bass_env,bass_target,room_lp,wet_lp,previous_input,master_env,last_print;
 int32_t current_rate_q16,read_q16;
 uint32_t pcm_length[2],write_count,read_start,read_end,read_position,room_pos,dub_pos;
 int32_t dub_lp;
 uint32_t bass_gate,bass_goal,pad_phase[8],pad_inc[8],ep_phase,ep_inc,ep_env,space_pos;
 int32_t pad_left,pad_right,air_env,air_gain,space_lp;
 uint8_t music_tick,event_count,event_cursor;
 b174_event event[12];
 uint8_t initialized,running,write_bank,ready_bank,print_ready,decimate,print_gate,scene_pending;
 uint8_t pending_scene,drum_mute,external_clock,manual_audition;
 uint32_t last_clock_frame,clock_ticks;
 int16_t pcm[2][B174_PCM];
 int16_t room[B174_ROOM];
 int16_t dub[B174_DUB];
 int16_t space[B174_SPACE];
} b174_engine;
void b174_init(b174_engine *e,uint32_t rate,uint32_t seed);
void b174_command(b174_engine *e,int command,int value);
void b174_key(b174_engine *e,int black,int index,int down);
void b174_hit(b174_engine *e,int role,int velocity);
void b174_render(b174_engine *e,int32_t *stereo,uint32_t frames);
int b174_status(const b174_engine *e,int field);
const char *b174_style_name(int style);
const char *b174_family_name(int family);
#endif
