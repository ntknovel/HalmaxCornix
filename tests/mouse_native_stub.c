/* Only ZMK dispatch is stubbed; the real mouse binding dispatcher is linked. */
#include <stdbool.h>
#include <drivers/behavior.h>
int zmk_behavior_invoke_binding(const struct zmk_behavior_binding *b,struct zmk_behavior_binding_event e,bool p){(void)b;(void)e;(void)p;return 0;}
