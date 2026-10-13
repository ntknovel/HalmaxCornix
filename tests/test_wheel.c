/* Run the actual production router against an explicit host ABI mirror. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <test_api.h>
#include "../src/behavior_cck_wheel.c"
static struct zmk_behavior_binding map[8][64];
static uint8_t order[8]={0,1,2,3,4,5,6,7};
static unsigned active=1, top=0, calls=0, assertions=0;
static int fault_press=0,fault_release=0;
static struct zmk_behavior_binding last;
static struct zmk_behavior_binding_event last_e;
static bool last_pressed;
#define CK(c) do {assert(c);assertions++;}while(0)
uint8_t zmk_keymap_highest_layer_active(void){return top;}
uint8_t zmk_keymap_layer_default(void){return 0;}
uint8_t zmk_keymap_layer_index_to_id(uint8_t i){return i<8?order[i]:UINT8_MAX;}
bool zmk_keymap_layer_active(uint8_t id){return id<8 && (active&(1u<<id));}
const struct zmk_behavior_binding *zmk_keymap_get_layer_binding_at_idx(uint8_t l,uint8_t p){return l<8&&p<64?&map[l][p]:NULL;}
int zmk_behavior_invoke_binding(const struct zmk_behavior_binding *b,struct zmk_behavior_binding_event e,bool p){last=*b;last_e=e;last_pressed=p;calls++;return p?fault_press:fault_release;}
static void reset(void){memset(map,0,sizeof(map));memset(latches,0,sizeof(latches));active=1;top=0;calls=0;fault_press=fault_release=0;for(unsigned i=0;i<8;i++)order[i]=i;}
static struct zmk_behavior_binding router(unsigned slot){return (struct zmk_behavior_binding){.behavior_dev="cck_wheel",.param1=slot};}
static struct zmk_behavior_binding_event event(unsigned slot){return (struct zmk_behavior_binding_event){.position=64+(slot-60)/2,.timestamp=100,.source=0};}
int main(void){
 struct behavior_parameter_metadata md;
 CK(api.get_parameter_metadata(NULL,&md)==0);CK(md.sets_len==1);CK(md.sets[0].param1_values[0].range.min==60);CK(md.sets[0].param1_values[0].range.max==63);CK(metadata(NULL,NULL)==-EINVAL);
 for(unsigned p=60;p<64;p++){
  reset();struct zmk_behavior_binding b=router(p);struct zmk_behavior_binding_event e=event(p);
  map[0][p]=(struct zmk_behavior_binding){.local_id=77,.behavior_dev="kp",.param1=p,.param2=0};
  CK(api.binding_pressed(&b,e)==0);CK(calls==1&&last_pressed&&last.param1==p);CK(last.local_id==77);CK(last_e.source==UINT8_MAX);CK(last_e.position==p);
  CK(on_pressed(&b,e)==-EBUSY); // never overwrite a pending release
  // change both keymap and active layer before the delayed release
  map[0][p].param1=999;map[1][p].behavior_dev="mo";active=3;top=1;e.timestamp=110;
  CK(on_released(&b,e)==0);CK(calls==2&&!last_pressed&&last.param1==p&&last_e.layer==0);CK(last_e.timestamp==110);
  CK(on_released(&b,e)==0&&calls==2);
 }
 reset();struct zmk_behavior_binding b=router(60);struct zmk_behavior_binding_event e=event(60);
 map[0][60]=(struct zmk_behavior_binding){.behavior_dev="kp",.param1=12};
 map[5][60]=(struct zmk_behavior_binding){.behavior_dev="st_arrow",.param1=80};active=33;top=5;
 CK(on_pressed(&b,e)==0&&last.param1==80&&last_e.layer==5);CK(on_released(&b,e)==0);
 // layer ORDER is not a layer ID
 order[1]=5;order[5]=1;top=1;CK(on_pressed(&b,e)==0&&last_e.layer==5);CK(on_released(&b,e)==0);
 map[5][60].behavior_dev="trans";CK(on_pressed(&b,e)==0&&last_e.layer==0&&last.param1==12);CK(on_released(&b,e)==0);
 map[5][60].behavior_dev="none";CK(on_pressed(&b,e)==0&&strcmp(last.behavior_dev,"none")==0);CK(on_released(&b,e)==0);
 map[5][60].behavior_dev="cck_wheel";unsigned n=calls;CK(on_pressed(&b,e)==-ELOOP&&calls==n);
 b.param1=59;CK(on_pressed(&b,e)==-EINVAL);b.param1=64;CK(on_pressed(&b,e)==-EINVAL);b.param1=60;b.param2=1;CK(on_pressed(&b,e)==-EINVAL);b.param2=0;
 e.position=60;CK(on_pressed(&b,e)==-EINVAL);e.position=65;CK(on_pressed(&b,e)==-EINVAL);e=event(60);
 reset();map[0][60]=(struct zmk_behavior_binding){.behavior_dev="kp",.param1=91};fault_press=-ENODEV;
 CK(on_pressed(&b,e)==-ENODEV);CK(calls==2&&!last_pressed&&!latches[0].active);CK(on_released(&b,e)==0&&calls==2);
 fault_press=0;CK(on_pressed(&b,e)==0);fault_release=-EIO;CK(on_released(&b,e)==-EIO);CK(!latches[0].active);
 reset();CK(on_pressed(&b,e)==0&&calls==0);CK(on_released(&b,e)==0&&calls==0);
 // Long alternating sequence, no state leakage between directions/halves.
 for(unsigned p=60;p<64;p++)map[0][p]=(struct zmk_behavior_binding){.behavior_dev="kp",.param1=p};
 for(unsigned i=0;i<10000;i++){b=router(60+i%4);e=event(b.param1);CK(on_pressed(&b,e)==0);CK(last.param1==b.param1);CK(on_released(&b,e)==0);CK(!last_pressed);}
 printf("Production CCK wheel router: PASS (%u assertions, host ABI mirror; not actual sensors)\n",assertions);
}
