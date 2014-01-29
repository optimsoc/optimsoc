#include <spr-defs.h>
#include <or1k-support.h>
#include <stdint.h>

uint32_t optimsoc_critical_begin(void) {
    int sr = or1k_mfspr(SPR_SR);
    or1k_mtspr(SPR_SR,sr & ~(SPR_SR_TEE|SPR_SR_IEE));
    return sr & (SPR_SR_TEE|SPR_SR_IEE);
}

void optimsoc_critical_end(uint32_t restore) {
    or1k_mtspr(SPR_SR,or1k_mfspr(SPR_SR)|(restore & (SPR_SR_TEE|SPR_SR_IEE)));
}
