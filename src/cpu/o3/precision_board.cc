
#include "cpu/o3/precision_board.hh"

#include "config/the_isa.hh"

PrecisionBoard::PrecisionBoard(const std::string &_my_name,
                               unsigned _numPhysicalRegs)
    : _name(_my_name),
      regPrecBoard(_numPhysicalRegs),
      numPhysRegs(_numPhysicalRegs)

{
}
