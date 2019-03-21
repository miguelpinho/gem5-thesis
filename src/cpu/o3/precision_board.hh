
#ifndef __CPU_O3_PRECISION_BOARD_HH__
#define __CPU_O3_PRECISION_BOARD_HH__

#include <iostream>
#include <utility>
#include <vector>

#include "base/trace.hh"
#include "config/the_isa.hh"
#include "cpu/o3/comm.hh"
#include "cpu/precision_types.hh"
#include "debug/PrecBoard.hh"

/**
 * Implements a auxiliary structure to store the precision of the
 * physical registers of a certain class. The register indexing is
 * relative, as the register class is implied by the instance.
 *
 * @todo make template, to allow different types of definitions of
 * precision and/or types to be supplied
 */

class PrecisionBoard
{
  private:
    const std::string _name;

    std::vector<PrecStruct> regPrecBoard;

    unsigned numPhysRegs;

  public:
    PrecisionBoard(const std::string &_my_name,
                   unsigned _numPhysicalRegs);

    ~PrecisionBoard() {}

    std::string name() const { return _name; };

    PrecVal getPrecReg(PhysRegIdPtr phys_reg) const
    {
        DPRINTF(PrecBoard, "Getting precision of reg %i (%s)\n",
                phys_reg->index(), phys_reg->className());

        assert(phys_reg->classValue() == IntRegClass);
        assert(phys_reg->flatIndex() < numPhysRegs);

        // zero reg has fixed precision
        if (phys_reg->isZeroReg()) {
            DPRINTF(PrecBoard, "Reg %i (%s) is a zero register\n",
                    phys_reg->index(), phys_reg->className());

            return PrecZero;
        }

        /* assert(!regPrecBoard[phys_reg->flatIndex()].isNone()); */
        if (regPrecBoard[phys_reg->flatIndex()].isNone()) {
            DPRINTF(PrecBoard, "Reg %i (%s) had no assinged precision\n",
                    phys_reg->index(), phys_reg->className());
        }

        PrecVal prc = regPrecBoard[phys_reg->flatIndex()].get();

        return prc;
    }

    void setPrecReg(PhysRegIdPtr phys_reg, uint64_t val) {
        assert(phys_reg->flatIndex() < numPhysRegs);

        DPRINTF(PrecBoard, "Setting precision of reg %i (%s)\n",
                phys_reg->index(), phys_reg->className());

        // zero reg can never change precision
        if (phys_reg->isZeroReg()) {
            DPRINTF(PrecBoard, "Reg %i (%s) is a zero register\n",
                    phys_reg->index(), phys_reg->className());

            return;
        }

        regPrecBoard[phys_reg->flatIndex()].setByVal(val);
    }

    void clearPrecReg(PhysRegIdPtr phys_reg) {
        assert(phys_reg->flatIndex() < numPhysRegs);

        DPRINTF(PrecBoard, "Clearing precision of reg %i (%s)\n",
                phys_reg->index(), phys_reg->className());

        // zero reg always has the same precision
        if (phys_reg->isZeroReg()) {
            DPRINTF(PrecBoard, "Reg %i (%s) is a zero register\n",
                    phys_reg->index(), phys_reg->className());
            return;
        }

        regPrecBoard[phys_reg->flatIndex()].clear();
    }

};

#endif // __CPU_O3_PRECISION_BOARD_HH__
