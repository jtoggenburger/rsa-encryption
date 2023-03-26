#include "randstate.h"

#include <gmp.h>

gmp_randstate_t state;

//initialize the random state using seed
void randstate_init(uint64_t seed) {
    gmp_randinit_mt(state); //initializes it
    gmp_randseed_ui(state, seed); //seeds it
    return;
}

//delete the random state
void randstate_clear(void) {
    gmp_randclear(state); //deallocates the random variable
    return;
}
