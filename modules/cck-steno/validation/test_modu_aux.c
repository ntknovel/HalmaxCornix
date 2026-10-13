/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <dt-bindings/zmk/keys.h>
#include <zephyr/kernel.h>
#include <cornix_steno/modu_aux.h>
#include <cornix_steno/modu_position.h>
#include <cornix_steno/engine.h>
#include <cornix_steno/roles.h>
#include <cornix_steno/dual.h>
#include <cornix_steno/output.h>
struct test_key_event { uint32_t key; bool state; int64_t ts; };
extern struct test_key_event test_key_events[];
extern size_t test_key_event_count;
void test_key_events_reset(void);
static size_t cases;
static void reset(bool steno) {
    cornix_steno_output_flush(); modu_steno_aux_reset();
    cornix_steno_dual_reset(); cornix_steno_engine_set_active(steno);
    test_key_events_reset(); test_time_set(1000);
}
static void drain(void) { for (unsigned i=0;i<2500;i++) test_time_advance(1); }
static void press(enum cornix_steno_role r, unsigned p) {
    assert(cornix_steno_engine_role_pressed(r,p,k_uptime_get())>=0);
}
static void release(enum cornix_steno_role r, unsigned p) {
    assert(cornix_steno_engine_role_released(r,p,k_uptime_get())>=0);
}
static void expect(const uint32_t *keys, size_t n) {
    if (test_key_event_count!=n*2) {
        fprintf(stderr,"case=%zu count=%zu expected=%zu\n",cases,test_key_event_count,n*2);
        for(size_t i=0;i<test_key_event_count;i++) fprintf(stderr,"event %zu: %x %d\n",i,test_key_events[i].key,test_key_events[i].state);
        assert(test_key_event_count==n*2);
    }
    for(size_t i=0;i<n;i++) {
        assert(test_key_events[i*2].key==keys[i] && test_key_events[i*2].state);
        assert(test_key_events[i*2+1].key==keys[i] && !test_key_events[i*2+1].state);
    }
    cases++;
}
static const struct modu_steno_aux_config cfgs[] = {
 {TILDE,GRAVE,ESC,false},
 {N1,LS(N1),F1,false},{N2,LS(N2),F2,false},{N3,LS(N3),F3,false},
 {N4,LS(N4),F4,false},{N5,LS(N5),F5,false},{N6,LS(N6),F6,false},
 {N7,LS(N7),F7,false},{N8,LS(N8),F8,false},{N9,LS(N9),F9,false},
 {N0,LS(N0),F10,false},{BSPC,BSPC,F11,false},{TAB,TAB,F12,true}
};
int main(void) {
    const enum cornix_steno_role markers[]={CST_R_SYMBOL_L,CST_R_SYMBOL_R,CST_R_VEXT_L,CST_R_VEXT_R};
    const unsigned pos[]={40,47,24,37};
    /* All 13 auxiliary keys x 16 marker subsets x both release orders. */
    for(unsigned c=0;c<13;c++) for(unsigned mask=0;mask<16;mask++) for(unsigned order=0;order<2;order++) {
        reset(true);
        for(unsigned j=0;j<4;j++) if(mask&(1u<<j))press(markers[j],pos[j]);
        assert(modu_steno_aux_pressed(&cfgs[c],c,false,false,k_uptime_get())==0);
        if(order)for(unsigned j=0;j<4;j++)if(mask&(1u<<j))release(markers[j],pos[j]);
        assert(modu_steno_aux_released(c)==0);
        if(!order)for(unsigned j=0;j<4;j++)if(mask&(1u<<j))release(markers[j],pos[j]);
        drain();
        uint32_t expected=(mask&12)?cfgs[c].function_key:(mask&3)?cfgs[c].symbol_key:cfgs[c].normal_key;
        expect(&expected,1);
    }
    /* Base uses ordinary Fn/Symbol layers, not steno role state. */
    for(unsigned c=0;c<13;c++)for(unsigned flags=0;flags<4;flags++) {
        reset(false);assert(!modu_steno_aux_pressed(&cfgs[c],c,flags&1,flags&2,k_uptime_get()));
        assert(!modu_steno_aux_released(c));drain();
        uint32_t expected=(flags&1)?cfgs[c].function_key:(flags&2)?cfgs[c].symbol_key:cfgs[c].normal_key;
        expect(&expected,1);
    }
    /* Pending Korean stroke is emitted before the new digit; late releases do not replay it. */
    reset(true);press(CST_R_I_G,4);press(CST_R_V_A,46);
    assert(!modu_steno_aux_pressed(&cfgs[1],1,false,false,k_uptime_get()));
    release(CST_R_I_G,4);release(CST_R_V_A,46);
    assert(!modu_steno_aux_released(1));drain();
    const uint32_t ga1[]={R,K,N1};expect(ga1,3);
    /* A borrowed vowel marker can immediately participate in another vowel chord. */
    reset(true);press(CST_R_VEXT_L,24);
    assert(!modu_steno_aux_pressed(&cfgs[1],1,false,false,k_uptime_get()));
    assert(!modu_steno_aux_released(1));drain();
    press(CST_R_I_G,4);press(CST_R_V_A,46);release(CST_R_I_G,4);release(CST_R_V_A,46);release(CST_R_VEXT_L,24);drain();
    const uint32_t fya[]={F1,R,I};expect(fya,3);
    /* Original Symbol+Q number stream remains available after a top-row symbol. */
    reset(true);press(CST_R_SYMBOL_L,40);
    assert(!modu_steno_aux_pressed(&cfgs[1],1,false,false,k_uptime_get()));
    assert(!modu_steno_aux_released(1));drain();
    press(CST_R_I_B,1);release(CST_R_I_B,1);release(CST_R_SYMBOL_L,40);drain();
    const uint32_t ex1[]={LS(N1),N1};expect(ex1,2);
    /* Solo dual vowels before digits preserve 500ms policy, without trailing Enter/Space. */
    for(unsigned longhold=0;longhold<2;longhold++) {
        reset(true);cornix_steno_dual_pressed(44,CST_DUAL_VOWEL,CST_R_V_EO,SPACE,0,k_uptime_get());
        test_time_advance(longhold?500:50);
        assert(!modu_steno_aux_pressed(&cfgs[1],1,false,false,k_uptime_get()));
        assert(!modu_steno_aux_released(1));cornix_steno_dual_released(44,k_uptime_get());drain();
        const uint32_t expected[]={longhold?J:SPACE,N1};expect(expected,2);
    }
    /* Backspace remains held for host repeat; release is not converted to F11. */
    reset(true);assert(!modu_steno_aux_pressed(&cfgs[11],11,false,false,k_uptime_get()));
    drain();assert(test_key_event_count==1 && test_key_events[0].state);
    press(CST_R_VEXT_R,37);assert(!modu_steno_aux_released(11));
    /* Do not release the unused marker: this case tests only key identity. */
    drain();const uint32_t bs[]={BSPC};expect(bs,1);
    /* Flush after real DOWN releases it; late physical release is a safe no-op. */
    reset(true);press(CST_R_VEXT_R,37);assert(!modu_steno_aux_pressed(&cfgs[1],1,false,false,k_uptime_get()));
    test_time_advance(0);cornix_steno_output_flush();modu_steno_aux_reset();
    assert(!modu_steno_aux_released(1));drain();const uint32_t f1[]={F1};expect(f1,1);
    /* FIFO capacity reserves UP. Filled queue rejects whole generated sequences. */
    reset(false);assert(!cornix_steno_output_enqueue_event(F1,true));
    uint32_t filler[254];for(unsigned i=0;i<254;i++)filler[i]=A;
    assert(!cornix_steno_output_enqueue_sequence(filler,254));
    assert(cornix_steno_output_enqueue(B)==-ENOSPC);
    assert(!cornix_steno_output_enqueue_event(F1,false));
    cornix_steno_output_flush();assert(!cornix_steno_output_pending());cases++;
    /* Physical index map is total for real switches and excludes all virtual controls. */
    uint64_t seen=0;for(unsigned i=0;i<60;i++){unsigned p=modu_steno_engine_position(i);assert(p<64);assert(!(seen&(UINT64_C(1)<<p)));seen|=UINT64_C(1)<<p;}
    for(unsigned i=60;i<75;i++)assert(modu_steno_engine_position(i)==UINT32_MAX);
    printf("CCK / MODU auxiliary integration: PASS (%zu cases; 416 selector/release-order cases, 52 Base-layer cases, FIFO/dual/rollover/flush checks)\n",cases);
    return 0;
}
