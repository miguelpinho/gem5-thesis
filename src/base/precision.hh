/// MIGUELP 10-oct-2018 #DAB3E238# BEGIN ///
#ifndef __BASE_PRECISION_HH__
#define __BASE_PRECISION_HH__

#include <inttypes.h>

/**
 * Returns the unsigned precision, in bits, of an integer value.
 * The unsigned precision is measured as the position of the most
 * significant 1 bit. It returns zero for val == 0.
 *
 * @param val: up to 64-bit integer value
 *
 * @return precision in bits (0 to 64 unsigned)
 */
unsigned unsignedIntPrecision(uint64_t val);

/**
 * Returns the signed precision, in bits, of an integer value.
 * The signed precision is measured as the position (starting
 * in 1) of the least significant leading 0 or 1 bit.
 *
 * @param val 64-bit integer value
 *
 * @return precision in bits (1 to 64, unsigned)
 */
unsigned signedIntPrecision(uint64_t val);

/**
 * Returns the bit blocks needed to represent an integer value.
 * For a given block size, calculates the minimal block count that
 * keeps the precision, according to this module's signed int
 * precision.
 *
 * @param val 64-bit integer value
 * @param block size of the blocks in bits
 *
 * @return count of needed blocks
 */
unsigned blockSIntPrecision(uint64_t val, unsigned block);

/**
 * Returns the base 2 log of the blocks needed for an integer value.
 * For a given a block size, calculates the power of two that is enough
 * to represent that value keeping the precision, according to this
 * module's signed int precision.
 *
 * @param val 64-bit integer value
 * @param block size of the blocks in bits
 *
 * @return smallest power of two of blocks needed
 */
unsigned logSIntPrecision(uint64_t val, unsigned block);

#endif // __BASE_PRECISION_HH__
/// MIGUELP 10-oct-2018 #DAB3E238# END ///
