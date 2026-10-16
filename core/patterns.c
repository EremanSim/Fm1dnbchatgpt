/* SPDX-License-Identifier: GPL-3.0-only */
const char *b174_style_name(int n){static const char *const names[]={"DNB","JUNGLE","LIQUID"};return names[b174_clamp(n,0,2)];}
const char *b174_family_name(int n){static const char *const names[]={"TWO STEP","AMEN STYLE","THINK STYLE","FUNKY GHOST","APACHE STYLE","ROLLER","HALFTIME"};return names[b174_clamp(n,0,6)];}
static void b174_step(b174_engine *e){
 unsigned s=e->step&15u,bar=e->bar&3u,f=e->set.family;
 int fill=e->fill_until&&e->frame<e->fill_until;
 uint32_t rnd=b174_random(e);unsigned chance=e->set.complexity;
 if(s==0){
  static const uint16_t basshz[12]={33,35,37,39,41,44,46,49,52,55,58,62};
  unsigned hz=basshz[e->set.root%12u];
  if((bar==2u||bar==3u)&&e->set.style==2)hz=hz*3u/2u;
  e->bass_inc=hz*97391u;e->bass_target=e->set.bass?5000:0;
 }
 if(f==6){if(s==0||s==10)b174_hit(e,B174_KICK,118);if(s==8)b174_hit(e,B174_SNARE,121);}
 else{
  int kick=(s==0||s==10||(f==1&&(s==6||s==11))||(f==4&&s==7));
  int snare=(s==4||s==12);
  if(f==1&&bar>=2){snare=(s==4||s==14);kick=(s==0||s==7||s==10);}
  if(kick)b174_hit(e,B174_KICK,s==0?124:103);
  if(snare)b174_hit(e,B174_SNARE,120);
  if((s==3||s==7||s==15)&&(f==1||f==2||f==3||e->set.style==1))b174_hit(e,B174_GHOST,35+(int)(rnd%24));
  if(f==1&&!(s&1u))b174_hit(e,B174_RIDE,s&2u?66:80);
  if(f==4&&(s==5||s==13||s==15))b174_hit(e,B174_TOM,70+(int)(rnd%30));
  if(f==2&&(s&1u))b174_hit(e,B174_PERC,50+(int)(rnd%22));
 }
 if(f!=1){if(!(s&1u)||f==5)b174_hit(e,B174_HAT,(s&2u)?60:81);if(s==14&&f!=6)b174_hit(e,B174_OPEN,62);}
 if(s!=4&&s!=12&&(rnd%128u)<chance/3u&&s&1u)b174_hit(e,B174_GHOST,28+(int)((rnd>>8)%24));
 if(fill||((e->held&(1u<<3))&&s&1u))b174_hit(e,B174_GHOST,45+(int)(s*3));
 if((e->set.roll>0)&&s>=14&&((rnd>>16)%128)<e->set.roll)b174_hit(e,B174_SNARE,74);
 if(e->set.bass&&(s==0||s==6||s==10)){e->bass_target=5200;e->bass_env=0;}
 if(s==0)e->print_gate=1;
}
