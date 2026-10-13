#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <zephyr/kernel.h>
#include <cornix_steno/modu_position.h>
#include <cornix_steno/roles.h>
#include <cornix_steno/decoder.h>
#include <cornix_steno/engine.h>
#include <cornix_steno/dual.h>
#include <dt-bindings/zmk/keys.h>
#include "cck_roles.h"
#include "role_position_map.h"
extern uint32_t test_output_keys[];extern size_t test_output_count;
void test_output_reset(void);
static unsigned tests;
static uint8_t physical_for(enum cornix_steno_role r){for(unsigned i=0;i<42;i++)if(rolemap[i].role==r)return rolemap[i].physical;assert(0);return 0;}
static void press(enum cornix_steno_role r,int64_t t){test_time_set(t);assert(cornix_steno_engine_role_pressed(r,modu_steno_engine_position(physical_for(r)),t)>=0);}
static void release(enum cornix_steno_role r,int64_t t){test_time_set(t);assert(cornix_steno_engine_role_released(r,modu_steno_engine_position(physical_for(r)),t)>=0);}
static void output(const uint32_t *s,size_t n){assert(test_output_count==n);assert(!n||!memcmp(s,test_output_keys,n*sizeof(uint32_t)));test_output_reset();tests++;}
int main(void){
 uint64_t seen=0;
 for(unsigned p=0;p<60;p++){unsigned e=modu_steno_engine_position(p);assert(e<64);assert(!(seen&(UINT64_C(1)<<e)));seen|=UINT64_C(1)<<e;}
 for(unsigned p=60;p<100;p++)assert(modu_steno_engine_position(p)==UINT32_MAX);
 for(unsigned i=0;i<42;i++)assert(modu_steno_engine_position(rolemap[i].physical)==rolemap[i].engine && rolemap[i].engine==cst_test_position_for_role(rolemap[i].role));
 // Compare the decoder on every single, pair and triple using actual CCK coordinates.
 for(unsigned a=0;a<42;a++)for(unsigned b=a;b<42;b++)for(unsigned c=b;c<42;c++){
  uint64_t mask=(UINT64_C(1)<<rolemap[a].role)|(UINT64_C(1)<<rolemap[b].role)|(UINT64_C(1)<<rolemap[c].role);
  uint64_t actual=(UINT64_C(1)<<modu_steno_engine_position(rolemap[a].physical))|(UINT64_C(1)<<modu_steno_engine_position(rolemap[b].physical))|(UINT64_C(1)<<modu_steno_engine_position(rolemap[c].physical));
  uint64_t expected=(UINT64_C(1)<<rolemap[a].engine)|(UINT64_C(1)<<rolemap[b].engine)|(UINT64_C(1)<<rolemap[c].engine);
  struct cornix_steno_decoded x={0},y={0};int u=cornix_steno_decode(mask,actual,&x),v=cornix_steno_decode(mask,expected,&y);
  assert(u==v&&x.kind==y.kind&&x.key_count==y.key_count&&!memcmp(x.keys,y.keys,sizeof(x.keys)));tests++;
 }
 cornix_steno_engine_set_active(true);test_output_reset();
 press(CST_R_I_B,1000);press(CST_R_V_A,1005);press(CST_R_F_G,1010);
 release(CST_R_F_G,1020);test_time_advance(300);output(NULL,0);release(CST_R_I_B,1330);release(CST_R_V_A,1340);output((uint32_t[]){Q,K,R},3);
 press(CST_R_F_B,2000);press(CST_R_V_U,2010);release(CST_R_F_B,2020);release(CST_R_V_U,2030);output((uint32_t[]){Q,N},2);
 press(CST_R_I_DOUBLE,2100);press(CST_R_F_B,2110);press(CST_R_V_U,2120);release(CST_R_V_U,2130);release(CST_R_F_B,2140);release(CST_R_I_DOUBLE,2150);output((uint32_t[]){LS(Q),N},2);
 press(CST_R_I_B,2200);press(CST_R_F_DOUBLE,2210);press(CST_R_V_A,2220);release(CST_R_V_A,2230);release(CST_R_I_B,2240);release(CST_R_F_DOUBLE,2250);output((uint32_t[]){LS(Q),K},2);
 printf("CCK 60+4 coordinate / actual decoder integration: PASS (%u cases)\n",tests);
}
