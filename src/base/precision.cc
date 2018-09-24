/// MIGUELP 13-sep-2018 #CDD27DFF# BEGIN ///

#include "base/precision.hh"

unsigned
intPrecision(uint64_t val)
{
    if (val == 0) return 0;

    unsigned prc = 1;
    uint64_t aux = val;

    if (aux >> 32) { prc += 32; aux = aux >> 32; }
    if (aux >> 16) { prc += 16; aux = aux >> 16; }
    if (aux >> 8) { prc += 8; aux = aux >> 8; }
    if (aux >> 4) { prc += 4; aux = aux >> 4; }
    if (aux >> 2) { prc += 2; aux = aux >> 2; }
    if (aux >> 1) { prc++; }

    return prc;
}

/// MIGUELP 13-sep-2018 #CDD27DFF# END ///
