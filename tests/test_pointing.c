/* Host integration test: actual engine, dual vowel, mouse binding dispatcher,
 * and input processor. Only Zephyr time/locks and ZMK transport are substitutes. */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <dt-bindings/zmk/keys.h>
#include <cornix_steno/roles.h>
#include <cornix_steno/engine.h>
#include <cornix_steno/dual.h>
#include <cornix_steno/tab.h>
#include <cornix_steno/pointer.h>
#include <cornix_steno/modu_position.h>
#include "cck_roles.h"
#include "../src/input_processor_cck_scroll.c"
#include <drivers/behavior.h>

struct test_key_event {uint32_t key;bool state;int64_t ts;};
extern struct test_key_event test_key_events[];
extern size_t test_key_event_count, test_output_count;
extern uint32_t test_output_keys[];
void test_key_events_reset(void);void test_output_reset(void);
struct mouse_event {struct zmk_behavior_binding binding;struct zmk_behavior_binding_event event;bool pressed;};
static struct mouse_event mouse_events[64];static size_t mouse_count;
static int fail_next;
static uint8_t highest = 5;
uint8_t zmk_keymap_highest_layer_active(void){return highest;}
int zmk_behavior_invoke_binding(const struct zmk_behavior_binding *b,struct zmk_behavior_binding_event e,bool p){
 assert(mouse_count<64);mouse_events[mouse_count++]=(struct mouse_event){*b,e,p};
 if(fail_next){int x=fail_next;fail_next=0;return x;}return 0;
}
static struct cck_scroll_data scroll_data;
static struct device proc={.data=&scroll_data,.api=&api};
static int64_t ts=1000;static unsigned cases;
static void clear_logs(void){test_output_reset();test_key_events_reset();mouse_count=0;fail_next=0;}
static void reset(void){cornix_steno_dual_reset();cornix_steno_engine_set_active(true);cornix_steno_tab_reset();clear_logs();highest=5;memset(&scroll_data,0,sizeof(scroll_data));ts+=2000;test_time_set(ts);}
static uint32_t role_pos(enum cornix_steno_role r){for(unsigned i=0;i<42;i++)if(rolemap[i].role==r)return rolemap[i].engine;assert(0);return 0;}
static void press(enum cornix_steno_role r){ts+=10;test_time_set(ts);uint32_t p=role_pos(r);
 if(r==CST_R_V_EO||r==CST_R_V_U){assert(cornix_steno_dual_pressed(p,CST_DUAL_VOWEL,r,r==CST_R_V_EO?SPACE:ENTER,0,ts)>=0);}
 else{cornix_steno_dual_notify_other_press(p,ts);assert(cornix_steno_engine_role_pressed(r,p,ts)>=0);}}
static void release(enum cornix_steno_role r){ts+=10;test_time_set(ts);uint32_t p=role_pos(r);
 if(r==CST_R_V_EO||r==CST_R_V_U)assert(cornix_steno_dual_released(p,ts)>=0);
 else assert(cornix_steno_engine_role_released(r,p,ts)>=0);}
static struct input_event event(uint8_t type,uint16_t code,int32_t value,bool sync){
 struct input_event e={.dev=&proc,.type=type,.code=code,.value=value,.sync=sync};
 assert(cck_scroll_handle(&proc,&e,0,0,NULL)==0);return e;}
static void no_text(void){assert(test_output_count==0);}
static void text(const uint32_t *v,size_t n){assert(test_output_count==n);assert(!n||memcmp(v,test_output_keys,n*sizeof(*v))==0);}
int main(void){
 assert(modu_steno_engine_position(54)==44);
 const enum cornix_steno_role targets[]={CST_R_F_R,CST_R_F_N,CST_R_F_NG,CST_R_F_D};
 const uint32_t arrows[]={LEFT,RIGHT,DOWN,UP};
 for(int a=0;a<2;a++)for(int k=0;k<4;k++)for(int first=0;first<2;first++){
  reset();enum cornix_steno_role anchor=a?CST_R_V_A:CST_R_V_I;
  press(anchor);press(targets[k]);assert(test_key_event_count==1&&test_key_events[0].state&&test_key_events[0].key==arrows[k]);
  if(first){release(anchor);release(targets[k]);}else{release(targets[k]);release(anchor);}
  assert(test_key_event_count==2&&!test_key_events[1].state&&test_key_events[1].key==arrows[k]);no_text();cases++;
 }
 // All five mouse targets, both release orders, repeat and drag.
 const enum cornix_steno_role mt[]={CST_R_F_R,CST_R_F_N,CST_R_F_CH,CST_R_F_D,CST_R_F_NG};
 const uint32_t mp[]={1,2,4,12,0xfff4};
 for(int k=0;k<5;k++)for(int first=0;first<2;first++)for(int repeat=0;repeat<3;repeat++){
  reset();press(CST_R_V_EO);
  for(int n=0;n<=repeat;n++){
   clear_logs();press(mt[k]);assert(mouse_count==1&&mouse_events[0].pressed);
   assert(!strcmp(mouse_events[0].binding.behavior_dev,k<3?"mkp":"msc"));
   assert(mouse_events[0].binding.param1==mp[k]);
   if(first&&n==repeat)release(CST_R_V_EO);
   release(mt[k]);assert(mouse_count==2&&!mouse_events[1].pressed);
   assert(mouse_events[1].binding.param1==mp[k]);no_text();assert(test_key_event_count==0);
  }
  if(!first)release(CST_R_V_EO);no_text();cases++;
 }
 // Simultaneous five targets; anchor release must release ALL five, not four.
 reset();press(CST_R_V_EO);for(int k=0;k<5;k++)press(mt[k]);assert(mouse_count==5);
 release(CST_R_V_EO);assert(mouse_count==10);for(int k=0;k<5;k++)release(mt[k]);assert(mouse_count==10);no_text();cases++;
 // Standalone EO short and long retain their previous semantics.
 reset();press(CST_R_V_EO);release(CST_R_V_EO);text((uint32_t[]){SPACE},1);cases++;
 reset();press(CST_R_V_EO);ts+=500;release(CST_R_V_EO);text((uint32_t[]){J},1);cases++;
 // No movement or zero-only notifications do not swallow the Space tap.
 reset();press(CST_R_V_EO);event(INPUT_EV_REL,INPUT_REL_X,0,false);event(INPUT_EV_REL,INPUT_REL_Y,0,true);release(CST_R_V_EO);text((uint32_t[]){SPACE},1);cases++;
 // Scroll conversion per axis; no pointer leak, no Space/direct vowel on release.
 for(int axis=0;axis<2;axis++)for(int sign=-1;sign<=1;sign+=2){
  reset();press(CST_R_V_EO);
  struct input_event e=event(INPUT_EV_REL,axis?INPUT_REL_Y:INPUT_REL_X,sign*70,true);
  assert(e.code==(axis?INPUT_REL_WHEEL:INPUT_REL_HWHEEL)&&e.value==-sign&&e.sync);
  release(CST_R_V_EO);no_text();assert(test_key_event_count==0);cases++;
 }
 // Fractional remainder, across 3 reports; reset between two separate gestures.
 reset();press(CST_R_V_EO);struct input_event e=event(INPUT_EV_REL,INPUT_REL_Y,30,true);assert(e.value==0&&e.code==INPUT_REL_WHEEL);
 e=event(INPUT_EV_REL,INPUT_REL_Y,30,true);assert(e.value==0);e=event(INPUT_EV_REL,INPUT_REL_Y,20,true);assert(e.value==-1);
 release(CST_R_V_EO);no_text();press(CST_R_V_EO);e=event(INPUT_EV_REL,INPUT_REL_Y,60,true);assert(e.value==0);release(CST_R_V_EO);no_text();cases++;
 // Consume even a used sub-tick gesture, not only a full scroll step.
 reset();press(CST_R_V_EO);e=event(INPUT_EV_REL,INPUT_REL_Y,1,true);release(CST_R_V_EO);no_text();cases++;
 // Per-sample latch: releasing EO between X and Y still gives one complete scroll frame.
 reset();press(CST_R_V_EO);e=event(INPUT_EV_REL,INPUT_REL_X,70,false);assert(e.code==INPUT_REL_HWHEEL&&e.value==-1);
 release(CST_R_V_EO);e=event(INPUT_EV_REL,INPUT_REL_Y,70,true);assert(e.code==INPUT_REL_WHEEL&&e.value==-1);
 e=event(INPUT_EV_REL,INPUT_REL_Y,70,true);assert(e.code==INPUT_REL_Y&&e.value==70);no_text();cases++;
 // Ordinary consonant-first and non-target-first chords cannot turn into mouse mode.
 reset();press(CST_R_I_B);press(CST_R_V_EO);e=event(INPUT_EV_REL,INPUT_REL_Y,140,true);assert(e.code==INPUT_REL_Y);
 release(CST_R_I_B);release(CST_R_V_EO);text((uint32_t[]){Q,J},2);cases++;
 reset();press(CST_R_V_EO);press(CST_R_I_B);e=event(INPUT_EV_REL,INPUT_REL_Y,140,true);assert(e.code==INPUT_REL_Y);
 release(CST_R_I_B);release(CST_R_V_EO);text((uint32_t[]){Q,J},2);cases++;
 // Mouse mode followed by unrelated role is consumed, not appended as a syllable.
 reset();press(CST_R_V_EO);event(INPUT_EV_REL,INPUT_REL_Y,70,true);press(CST_R_I_B);release(CST_R_I_B);release(CST_R_V_EO);no_text();cases++;
 // Existing pointer, Scroll/Snipe output and SELECT higher layer are not converted.
 for(int layer=0;layer<8;layer++)if(layer!=5){reset();highest=layer;press(CST_R_V_EO);e=event(INPUT_EV_REL,INPUT_REL_Y,70,true);assert(e.code==INPUT_REL_Y&&e.value==70);release(CST_R_V_EO);text((uint32_t[]){SPACE},1);cases++;}
 reset();press(CST_R_V_EO);e=event(INPUT_EV_REL,INPUT_REL_WHEEL,2,true);assert(e.code==INPUT_REL_WHEEL&&e.value==2);release(CST_R_V_EO);text((uint32_t[]){SPACE},1);cases++;
 // Cancel / mode exit releases mouse hold without a text tap.
 reset();press(CST_R_V_EO);press(CST_R_F_R);cornix_steno_dual_reset();cornix_steno_engine_cancel_pending();assert(mouse_count==2&&!mouse_events[1].pressed);release(CST_R_F_R);release(CST_R_V_EO);no_text();cases++;
 reset();press(CST_R_V_EO);press(CST_R_F_N);cornix_steno_engine_set_active(false);assert(mouse_count==2);release(CST_R_V_EO);no_text();cases++;
 // Unknown args and failed press never latch an unbalanced mouse output.
 reset();assert(cck_steno_mouse_emit(0,true,ts)<0);assert(cck_steno_mouse_emit(6,true,ts)<0);
 fail_next=-5;assert(cck_steno_mouse_emit(CCK_MOUSE_LEFT,true,ts)==-5);assert(mouse_count==2);cck_steno_mouse_emit(CCK_MOUSE_LEFT,false,ts);assert(mouse_count==2);cases++;
 // Valid values produce bounded scroll output and no undefined integer operations.
 int32_t extremes[]={INT32_MIN,INT32_MAX,-8190,8190,-70,70,-1,1};
 for(unsigned i=0;i<sizeof(extremes)/sizeof(*extremes);i++){reset();press(CST_R_V_EO);e=event(INPUT_EV_REL,INPUT_REL_Y,extremes[i],true);assert(e.value>=-127&&e.value<=127&&e.code==INPUT_REL_WHEEL);release(CST_R_V_EO);no_text();cases++;}
 // 2000 independent randomized mouse sessions: matched down/up and no residual text.
 uint32_t seed=7;
 for(int i=0;i<2000;i++){reset();seed=seed*1664525u+1013904223u;int k=(seed>>8)%5;press(CST_R_V_EO);press(mt[k]);if(seed&1){release(CST_R_V_EO);release(mt[k]);}else{release(mt[k]);release(CST_R_V_EO);}assert(mouse_count==2&&!mouse_events[1].pressed);no_text();cases++;}
 printf("CCK v1.1.1 actual engine+dual+mouse+input-processor host integration: %u cases PASS\n",cases);
 return 0;
}
