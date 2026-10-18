/* SPDX-License-Identifier: GPL-3.0-only WITH GPL-3.0-linking-exception
 * Copyright (c) 2026 M-VAVE. Extracted from app/presets.c at b178ac43c25f010c60a878056fb5331fe693fd62.
 * Modification: standalone FM recipes; unrelated project code removed. */
#include "engine.h"
#include <string.h>
enum {FM1_FM_PROGRAMS=16,ORGAN=2,WARM_PAD=4,REED=5,STRINGS=12};
static FM1_DSP_DATA const char fm_names[FM1_FM_PROGRAMS][11] = {
    "NEON KEYS ","ORBIT BELL","SOFT ORGAN","ROUND BASS","WARM AIR  ","WOOD REED ",
    "SOFT EP   ","DRY CLAV  ","FM GUITAR ","MARIMBA   ","VIBES     ","SOFT BRASS",
    "FM STRINGS","SUB BASS  ","METAL HIT ","SOFT LEAD "
};
const char *groove_fm_program_name(unsigned program)
{
    return program < FM1_FM_PROGRAMS ? fm_names[program] : "UNKNOWN   ";
}
/* Three independently voiced operator pairs: body, colour and transient.
 * Unequal carrier levels avoid three identical full-level voices stacking. */
/* Pack recipe fields without changing the serialized patch layout. */
struct fm_pair { uint8_t level, coarse : 2, detune : 4, mod, ratio, fine, decay, body; };
_Static_assert(sizeof(struct fm_pair) == 7, "compact recipes");
struct fm_voice {
    uint8_t attack, decay, settle, release, body, sustain, mod_attack, mod_release, feedback;
    struct fm_pair pair[3];
};
static FM1_DSP_DATA const struct fm_voice voices[FM1_FM_PROGRAMS] = {
    {96,52,30,62,84,0,99,66,0, {{98,1,7,54,1,0,65,38},{91,1,8,60,1,0,57,58},{79,1,6,48,7,0,80,0}}},
    {99,50,28,53,70,0,99,62,0, {{96,1,7,58,3,50,59,0},{82,2,7,48,7,0,68,0},{69,3,7,35,11,0,80,0}}},
    {95,65,45,78,99,99,95,78,0, {{90,1,7,0,1,0,65,0},{87,2,7,0,1,0,65,0},{74,3,7,0,1,0,65,0}}},
    {97,72,46,82,91,84,99,86,0, {{99,1,7,42,1,0,76,32},{74,2,7,32,1,0,82,20},{0,1,7,0,1,0,80,0}}},
    {53,47,36,52,96,92,49,55,0, {{90,1,5,42,1,0,55,82},{84,1,9,34,2,0,53,76},{73,2,7,27,1,0,52,78}}},
    {84,61,46,72,94,89,78,77,1, {{95,1,7,61,2,0,63,86},{82,1,8,38,1,0,68,75},{70,3,7,29,1,0,72,60}}},
    {94,49,28,60,86,0,96,64,0, {{98,1,7,44,1,0,60,35},{86,1,8,38,3,0,66,20},{72,2,6,31,1,0,72,0}}},
    {99,75,47,80,42,0,99,83,1, {{96,1,7,67,1,0,82,18},{81,2,7,55,2,0,86,0},{0,1,7,0,1,0,80,0}}},
    {97,57,32,66,69,0,99,74,0, {{97,1,7,55,1,0,72,18},{83,2,7,46,3,0,81,0},{66,3,7,38,5,0,85,0}}},
    {99,67,37,72,34,0,99,81,0, {{98,1,7,60,3,0,85,0},{74,2,7,46,5,0,88,0},{58,3,7,32,7,0,91,0}}},
    {98,52,25,55,78,0,99,65,0, {{94,1,7,56,4,0,76,0},{76,2,8,39,3,0,81,0},{66,3,6,28,2,0,84,0}}},
    {81,62,42,72,96,90,67,75,1, {{95,1,7,67,1,0,62,90},{84,1,8,46,1,0,65,83},{70,2,6,40,2,0,68,75}}},
    {61,48,39,54,96,94,57,58,0, {{89,1,5,54,1,0,55,89},{83,1,9,36,2,0,55,83},{72,2,7,30,4,0,58,76}}},
    {95,72,44,84,96,94,99,86,0, {{99,1,7,0,1,0,80,0},{0,1,7,0,1,0,80,0},{0,1,7,0,1,0,80,0}}},
    {99,65,40,72,48,0,99,76,0, {{89,1,7,65,7,31,67,0},{70,2,7,58,11,47,73,0},{56,3,7,48,13,79,78,0}}},
    {88,60,40,71,96,90,82,75,1, {{96,1,7,45,1,0,65,83},{80,1,8,31,2,0,70,72},{71,2,6,24,1,0,74,66}}}
};
FM1_DSP_CODE void groove_fm_program(uint8_t p[156], unsigned tone)
{
    tone %= FM1_FM_PROGRAMS;
    const struct fm_voice *sound = &voices[tone];
    memset(p, 0, 156);
    memcpy(p + 145, fm_names[tone], 10);
    for (unsigned op = 0; op < 6; ++op) {
        uint8_t *v = p + 21 * op;
        const struct fm_pair *pair = &sound->pair[op / 2];
        unsigned carrier = op & 1U;
        v[0] = carrier ? sound->attack : sound->mod_attack;
        v[1] = carrier ? sound->decay : pair->decay;
        v[2] = sound->settle; v[3] = carrier ? sound->release : sound->mod_release;
        v[4] = 99; v[5] = carrier ? sound->body : pair->body;
        v[6] = carrier ? sound->sustain : sound->sustain ? pair->body : 0;
        v[8] = 50; v[10] = carrier ? 8 : 18; v[13] = 1;
        v[15] = carrier ? 2 : 4;
        v[16] = carrier ? pair->level : pair->mod;
        v[18] = carrier ? pair->coarse : pair->ratio;
        v[19] = carrier ? 0 : pair->fine; v[20] = pair->detune;
        if (tone == ORGAN) {
            static FM1_DSP_DATA const uint8_t levels[6] = {89,77,84,68,59,52};
            static FM1_DSP_DATA const uint8_t ratios[6] = {1,2,3,4,6,8};
            v[16] = levels[op]; v[18] = ratios[op]; v[19] = 0;
            v[5] = v[6] = 99; v[15] = 1; v[20] = 7;
        }
    }
    for (unsigned i = 126; i < 130; ++i) p[i] = 99;
    for (unsigned i = 130; i < 134; ++i) p[i] = 50;
    p[134] = tone == ORGAN ? 31 : 4; p[135] = sound->feedback;
    p[136] = 1; p[137] = 28; p[138] = 35; p[141] = 1; p[142] = 4;
    if (tone == WARM_PAD || tone == STRINGS || tone == REED) { p[139] = 2; p[143] = 2; }
    p[144] = 24; p[155] = 63;
}
