
#ifndef __CPU__PRECISION_TYPES_HH__
#define __CPU__PRECISION_TYPES_HH__

#include <inttypes.h>

#include "base/precision.hh"

/**
 * Struct to store the precision of a register
 */
typedef uint8_t PrecVal;

const PrecVal PrecZero = (PrecVal) 1;

class PrecStruct {
  private:
    bool none;
    PrecVal prc;

  public:
    PrecStruct()
        : none(true), prc(PrecZero)
    {
    }

    ~PrecStruct() { }

    void clear() {
        none = true;
        prc = PrecZero;
    }

    bool isNone() const {
        return none;
    }

    void set(PrecVal precision) {
        prc = precision;
        none = false;
    }

    void setByVal(uint64_t val) {
        prc = blockSIntPrecision(val, 8);
        none = false;
    }

    PrecVal get() const {
        /* assert(!none); */

        return prc;
    }
};

#if 0
/**
 * Defines a precision concept, to be associated with the operands/registers
 * of a certain class type. Defines how the precision is calculated, how it
 * is stored...
 *
 * @todo make this a template class?
 */
class PrecDef {
  public:
    PrecVal CalculatePrc(uint64_t val);
}
#endif

#endif // __CPU__PRECISION_TYPES_HH__
