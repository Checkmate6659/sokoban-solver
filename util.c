#include "util.h"

uint64_t prng_state = 1234567890123456789; //a very uninteresting and boring seed (no idea if it is any good for zobrist keys tho)
uint64_t pseudo_rng() //a quick and dirty pseudorandom integer generator (xorshift64)
{
    prng_state ^= prng_state << 13;
    prng_state ^= prng_state >> 7;
    prng_state ^= prng_state << 17;

    return prng_state;
}
//NOTE using seed 0x123456789ABCDEF: there is something very weird that happens more times than expected: when
//viewing the decimal expansions of the hashes, usually there is at least 1 or 2 digits that are not present at all
