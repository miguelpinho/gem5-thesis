/// MIGUELP 13-sep-2018 #DAB3E238# BEGIN ///
#ifndef __BASE_PRECISION_HH__
#define __BASE_PRECISION_HH__

#include <inttypes.h>

/**
 * Returns the precision, in bits, of an integer value
 *
 * @param val: up to 64-bit integer value
 *
 * @return precision in bits (0 to 64 unsigned)
 */
unsigned intPrecision(uint64_t val);

#endif // __BASE_PRECISION_HH__
/// MIGUELP 13-sep-2018 #DAB3E238# END ///
