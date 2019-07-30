/// MPINHO 12-mar-2019 BEGIN ///
#ifndef __CPU_O3_WIDTH_DECODER_IMPL_HH__
#define __CPU_O3_WIDTH_DECODER_IMPL_HH__

#include "arch/arm/generated/decoder.hh" /// MPINHO 17-jul-2019 END ///
#include "arch/generic/vec_reg.hh"
#include "arch/utility.hh"
#include "base/bitfield.hh"
#include "base/logging.hh"
#include "base/resolution.hh"
#include "base/trace.hh"
#include "cpu/o3/comm.hh"
#include "cpu/o3/inst_queue.hh"
#include "cpu/o3/width_decoder.hh"
#include "cpu/reg_class.hh"
#include "debug/WidthDecoder.hh"
#include "debug/WidthDecoderDecode.hh"
#include "debug/WidthDecoderWidth.hh"
#include "enums/OpClass.hh"
#include "params/DerivO3CPU.hh"

template <class Impl>
WidthDecoder<Impl>::WidthDecoder()
    : iqPtr(NULL)
{}

/// MPINHO 11-may-2019 BEGIN ///
template <class Impl>
WidthDecoder<Impl>::WidthDecoder(O3CPU *cpu_ptr, DerivO3CPUParams *params)
    : _name(params->name + ".widthdecoder"),
      cpu(cpu_ptr),
      iqPtr(NULL),
      widthDef(params->widthDefinition),
      blockSize(params->widthBlockSize),
      packingPolicy(params->widthPackingPolicy)
{
    DPRINTF(WidthDecoder, "Creating WidthDecoder object.\n");

    /** Parameters */
    widthDef = params->widthDefinition;
    blockSize = params->widthBlockSize;
    packingPolicy = params->widthPackingPolicy;

    // blockSize must be a power of 2.
    if (!(blockSize && ((blockSize & (blockSize-1)) == 0))) {
        fatal("Block size (%u) must be a power of 2.",
              blockSize);
    }

    // Set width definition function.
    switch (widthDef) {
        case WidthDefinition::Unsigned :
            prcFunc = unsignedIntResolution;
            break;

        case WidthDefinition::Signed :
            prcFunc = signedIntResolution;
            break;

        default:
            panic("\"%s\" unimplemented width definition.",
                WidthDefinitionStrings[static_cast<int>(widthDef)]);
            break;
    }

    // Set width rounding function based on the block size.
    roundFunc = roundPrcBlock;
    roundedPrcFunc =
        std::bind(roundFunc, std::bind(prcFunc, std::placeholders::_1),
                  blockSize);

    // Set packing policy function.
    switch (packingPolicy) {
        case WidthPackingPolicy::Disabled :
            packingCriteria =
                [] (VecWidthCode a, VecWidthCode b) { return false; };
            break;

        case WidthPackingPolicy::Simple :
            packingCriteria = simplePacking;
            break;

        case WidthPackingPolicy::Optimal :
            packingCriteria = optimalPacking;
            break;

        default:
            panic("\"%s\" packing criteria is not implemented.",
                  WidthPackingPolicyStrings[static_cast<int>(packingPolicy)]);
            break;
    }

    DPRINTF(WidthDecoder, "\tWidth definition: %s.\n",
            WidthDefinitionStrings[static_cast<int>(widthDef)]);
    DPRINTF(WidthDecoder, "\tBlock size: %u (bits)).\n", blockSize);
    DPRINTF(WidthDecoder, "\tPacking policy: %s.\n",
            WidthPackingPolicyStrings[static_cast<int>(packingPolicy)]);
}
/// MPINHO 11-may-2019 END ///

template <class Impl>
void
WidthDecoder<Impl>::setCPU(O3CPU *cpu_ptr)
{
    cpu = cpu_ptr;
}

template <class Impl>
void
WidthDecoder<Impl>::setIQ(InstructionQueue<Impl> *iq_ptr)
{
    iqPtr = iq_ptr;
}

template <class Impl>
WidthDecoder<Impl>::~WidthDecoder() {}

template <class Impl>
void
WidthDecoder<Impl>::init(DerivO3CPUParams *params)
{
     DPRINTF(WidthDecoder, "Creating WidthDecoder object.\n");

    _name = csprintf("%s.widthDecoder", params->name);

    /// MPINHO 08-may-2019 BEGIN ///
    /** Parameters */
    widthDef = params->widthDefinition;
    blockSize = params->widthBlockSize;
    packingPolicy = params->widthPackingPolicy;

    // Set width definition function.
    switch (widthDef) {
    case WidthDefinition::Unsigned :
        prcFunc = unsignedIntResolution;
        break;

    case WidthDefinition::Signed :
        prcFunc = signedIntResolution;
        break;

    default:
        panic("\"%s\" unimplemented width definition.",
              WidthDefinitionStrings[static_cast<int>(widthDef)]);
        break;
    }

    // Set width rounding function based on the block size.
    roundFunc = roundPrcBlock;
    roundedPrcFunc =
        std::bind(roundFunc, std::bind(prcFunc, std::placeholders::_1),
                  blockSize);

    // Set packing policy function.
    switch (packingPolicy) {
        case WidthPackingPolicy::Disabled :
            packingCriteria =
                [] (VecWidthCode a, VecWidthCode b) { return false; };
            break;

        case WidthPackingPolicy::Simple :
            packingCriteria = simplePacking;
            break;

        case WidthPackingPolicy::Optimal :
            packingCriteria = optimalPacking;
            break;

        default:
            panic("\"%s\" packing criteria is not implemented.",
                  WidthPackingPolicyStrings[static_cast<int>(packingPolicy)]);
            break;
    }

    DPRINTF(WidthDecoder, "\tWidth definition: %s.\n",
            WidthDefinitionStrings[static_cast<int>(widthDef)]);
    DPRINTF(WidthDecoder, "\tBlock size: %u (bits)).\n", blockSize);
    DPRINTF(WidthDecoder, "\tPacking policy: %s.\n",
            WidthPackingPolicyStrings[static_cast<int>(packingPolicy)]);
    /// MPINHO 08-may-2019 END ///
}

/**
 * @todo Change to use resol granularity.
 */
template <class Impl>
VecWidthCode
WidthDecoder<Impl>::vecSrcRegWidthMask(DynInstPtr &inst,
                                       uint8_t q, uint8_t size,
                                       uint8_t op)
{
    VecWidthCode mask;

    if (size == 0) {
        // 16x8-bit or (8x8-bit)
        mask = getWidthVecReg<16, int8_t>(inst, (q ? 16 : 8), 8, op);
    } else if (size == 1) {
        // 8x16-bit or (4x16-bit)
        mask = getWidthVecReg<8, int16_t>(inst, (q ? 8 : 4), 16, op);
    } else if (size == 2) {
        // 4x32-bit or (2x32-bit)
        mask = getWidthVecReg<4, int32_t>(inst, (q ? 4 : 2), 32, op);
    } else if (size == 3) {
        // 2x64-bit (or 1x64-bit)
        mask = getWidthVecReg<2, int64_t>(inst, (q ? 2 : 1), 64, op);
    } else {
        panic("Unknown eSize %d.", size);
    }

    DPRINTF(WidthDecoderWidth, "Source operand %d mask is %s (eSize=%i).\n",
            op,
            mask.to_string(),
            size);

    return mask;
}

template <class Impl>
template <int Size, typename Elem>
VecWidthCode
WidthDecoder<Impl>::getWidthVecReg(DynInstPtr &inst, int nElem, int nBits,
                                   uint8_t op)
{
    assert(nElem <= Size);

    VecWidthCode mask(Size, nBits);

    // FIXME: this count as an invalid access to the register, in terms of
    // stats?? Create proxy access function?
    const VecRegT<Elem, Size, true> &vsrc =
        inst->readVecRegOperand(inst->staticInst.get(), op);

    for (size_t i = 0; i < nElem; i++)
    {
        int rsl = roundedPrcFunc((uint64_t) (int64_t) vsrc[i]);

        DPRINTF(WidthDecoderWidth, "    Vec Lane %i: val=%d, rsl=%d\n",
                i, (int) vsrc[i], rsl);

        assert(rsl <= nBits);

        mask.set(i, rsl);
    }

    return mask;
}

template <class Impl>
void
WidthDecoder<Impl>::addWidthInfo(DynInstPtr &inst)
{
    inst->setWidth(decode(inst));
}

template <class Impl>
bool
WidthDecoder<Impl>::isFuseVecType(DynInstPtr &inst)
{
    return inst->getWidth().isFuseType();
}

template <class Impl>
bool
WidthDecoder<Impl>::canFuseInst(DynInstPtr &inst1, DynInstPtr &inst2)
{
    WidthInfo inst1_width = inst1->getWidth();
    WidthInfo inst2_width = inst2->getWidth();

    DPRINTF(WidthDecoder, "Trying to fuse %s and %s.\n",
            inst1->staticInst->disassemble(inst1->instAddr()),
            inst2->staticInst->disassemble(inst2->instAddr()));

    // TODO: Use chosen packing.
    return inst1_width.canFuse(inst2_width, optimalPacking);
}

template <class Impl>
WidthInfo
WidthDecoder<Impl>::decode(DynInstPtr &inst)
{
    using namespace ArmISAInst;

    ArmISA::ExtMachInst machInst = inst->staticInst->machInst;

    if (ILLEGALEXEC == 0x0 && DECODERFAULT == 0x0 && THUMB == 0x0) {
        if (AARCH64 == 0x1) {
            if (bits(machInst, 27, 25) == 0x7) {
                // bit 27:25=111 -> AdvSimd
                if (bits(machInst, 28) == 0) {
                    if (bits(machInst, 31) == 0) {
                        // AdvSimd Vector inst.
                        DPRINTF(WidthDecoderDecode,
                                "AdvSimd Vector inst decoded: %s.\n",
                                inst->staticInst->disassemble(
                                    inst->instAddr()));

                        if (bits(machInst, 24) == 1) {
                            if (bits(machInst, 10) == 0) {
                                // Neon IndexedElem.
                                DPRINTF(WidthDecoderDecode,
                                        "Neon Vector IndexedElem"
                                        " inst decoded: %s.\n",
                                        inst->staticInst->disassemble(
                                            inst->instAddr()));
                                return(WidthInfo(WidthClass::NoInfo));
                            } else if (bits(machInst, 23) == 1) {
                                // Nop.
                                return(WidthInfo(WidthClass::NoInfo));
                            } else {
                                if (bits(machInst, 22, 19)) {
                                    // Neon ShiftByImm.
                                    DPRINTF(WidthDecoderDecode,
                                            "Neon Vector ShiftByImm"
                                            " inst decoded: %s.\n",
                                            inst->staticInst->disassemble(
                                                inst->instAddr()));
                                    return(WidthInfo(WidthClass::NoInfo));
                                } else {
                                    // Neon NeonModImm.
                                    DPRINTF(WidthDecoderDecode,
                                            "Neon Vector ModImm"
                                            " inst decoded: %s.\n",
                                            inst->staticInst->disassemble(
                                                inst->instAddr()));
                                    return(WidthInfo(WidthClass::NoInfo));
                                }
                            }
                        } else if (bits(machInst, 21) == 1) {
                            if (bits(machInst, 10) == 1) {
                                // Neon 3Same.
                                DPRINTF(WidthDecoderDecode,
                                        "Neon Vector 3Same"
                                        " inst decoded: %s.\n",
                                        inst->staticInst->disassemble(
                                            inst->instAddr()));
                                return decodeNeon3Same(inst);
                            } else if (bits(machInst, 11) == 0) {
                                // Neon 3Diff.
                                DPRINTF(WidthDecoderDecode,
                                        "Neon Vector 3Diff"
                                        " inst decoded: %s.\n",
                                        inst->staticInst->disassemble(
                                            inst->instAddr()));
                                return(WidthInfo(WidthClass::NoInfo));
                            } else if (bits(machInst, 20, 17) == 0x0) {
                                // Neon 2RegMisc.
                                DPRINTF(WidthDecoderDecode,
                                        "Neon Vector 2RegMisc"
                                        " inst decoded: %s.\n",
                                        inst->staticInst->disassemble(
                                            inst->instAddr()));
                                return(WidthInfo(WidthClass::NoInfo));
                            } else if (bits(machInst, 20, 17) == 0x8) {
                                // Neon AcrossLanes.
                                DPRINTF(WidthDecoderDecode,
                                        "Neon Vector AcrossLanes"
                                        " inst decoded: %s.\n",
                                        inst->staticInst->disassemble(
                                            inst->instAddr()));
                                return(WidthInfo(WidthClass::NoInfo));
                            }
                        } else if (bits(machInst, 24) ||
                                bits(machInst, 21) ||
                                bits(machInst, 15)) {
                            // Nop.
                        } else if (bits(machInst, 10) == 1) {
                            if (!bits(machInst, 23, 22)) {
                                // Neon Copy.
                                DPRINTF(WidthDecoderDecode,
                                        "Neon Vector Copy"
                                        " inst decoded: %s.\n",
                                        inst->staticInst->disassemble(
                                            inst->instAddr()));
                                return(WidthInfo(WidthClass::NoInfo));
                            }
                        } else if (bits(machInst, 29) == 1) {
                            // Neon Ext.
                            DPRINTF(WidthDecoderDecode,
                                    "Neon Ext inst decoded: %s.\n",
                                    inst->staticInst->disassemble(
                                        inst->instAddr()));
                            return(WidthInfo(WidthClass::NoInfo));
                        } else if (bits(machInst, 11) == 1) {
                            // Neon ZipUzpTrn.
                            DPRINTF(WidthDecoderDecode,
                                    "Neon Vector ZipUzpTrn"
                                    " inst decoded: %s.\n",
                                    inst->staticInst->disassemble(
                                        inst->instAddr()));
                            return(WidthInfo(WidthClass::NoInfo));
                        } else if (bits(machInst, 23, 22) == 0x0) {
                            // NeonTblTbx.
                            DPRINTF(WidthDecoderDecode,
                                    "Neon Vector TblTbx"
                                    " inst decoded: %s.\n",
                                    inst->staticInst->disassemble(
                                        inst->instAddr()));
                            return(WidthInfo(WidthClass::NoInfo));
                        }
                    }
                } else if (bits(machInst, 31) == 0) {
                    // AdvSimd Scalar inst.

                    DPRINTF(WidthDecoderDecode,
                            "AdvSimd Scalar inst decoded: %s.\n",
                            inst->staticInst->disassemble(
                                inst->instAddr()));
                    return(WidthInfo(WidthClass::NoInfo));
                } else {
                    // Other AdvSimd inst.
                    DPRINTF(WidthDecoderDecode,
                            "Other AdvSimd inst decoded: %s.\n",
                            inst->staticInst->disassemble(
                                inst->instAddr()));
                    return(WidthInfo(WidthClass::NoInfo));
                }
            }

            return(WidthInfo(WidthClass::NoInfo));
        }
    }

    DPRINTF(WidthDecoderDecode,
            "Non AARCH64 inst decoded: %s.\n",
            inst->staticInst->disassemble(inst->instAddr()));
    return(WidthInfo(WidthClass::NoInfo));
}

template <class Impl>
WidthInfo
WidthDecoder<Impl>::decodeNeon3Same(DynInstPtr &inst)
{
    using namespace ArmISAInst;

    ArmISA::ExtMachInst machInst = inst->staticInst->machInst;

    uint8_t q = bits(machInst, 30);
    uint8_t u = bits(machInst, 29);
    uint8_t size = bits(machInst, 23, 22);
    uint8_t opcode = bits(machInst, 15, 11);

    uint8_t size_q = (size << 1) | q;

    switch (opcode) {
        case 0x00:
            if (size != 0x3) {
                // UhaddDX, UhaddQX, ShaddDX, ShaddQX
                DPRINTF(WidthDecoderDecode,
                        "Neon HADD inst decoded: %s. Size: %d, Q: %d.\n",
                        inst->staticInst->disassemble(inst->instAddr()),
                        size, q);
                return(WidthInfo(WidthClass::SimdPackingAdd,
                                 widthOp2VectorRegl(inst, q, size, 2, 3),
                                 size));
            }
            break;
        case 0x01:
            if (size_q != 0x6) {
                // UqaddDX, UqaddQX, SqaddDX, SqaddQX
                DPRINTF(WidthDecoderDecode,
                        "Neon QADD inst decoded: %s. Size: %d, Q: %d.\n",
                        inst->staticInst->disassemble(inst->instAddr()),
                        size, q);
                return(WidthInfo(WidthClass::SimdPackingAdd,
                                widthOp2VectorRegl(inst, q, size, 2, 3),
                                size));
            }
            break;
        case 0x02:
            if (size != 0x3) {
                // UrhaddDX, UrhaddQX, SrhaddDX, SrhaddQX
                DPRINTF(WidthDecoderDecode,
                        "Neon RHADD inst decoded: %s. Size: %d, Q: %d.\n",
                        inst->staticInst->disassemble(inst->instAddr()),
                        size, q);
                return(WidthInfo(WidthClass::SimdPackingAdd,
                                 widthOp2VectorRegl(inst, q, size, 2, 3),
                                 size));
            }
            break;
        case 0x04:
            if (size != 0x3) {
                // UhsubDX, UhsubQX, ShsubDX, ShsubQX
                DPRINTF(WidthDecoderDecode,
                        "Neon HSUB inst decoded: %s. Size: %d, Q: %d.\n",
                        inst->staticInst->disassemble(inst->instAddr()),
                        size, q);
                return(WidthInfo(WidthClass::SimdPackingAdd,
                                widthOp2VectorRegl(inst, q, size, 2, 3),
                                size));
            }
            break;
        case 0x05:
            if (size_q != 0x6) {
                // UqsubDX, UqsubQX, SqsubDX, SqsubQX
                DPRINTF(WidthDecoderDecode,
                        "Neon QSUB inst decoded: %s. Size: %d, Q: %d.\n",
                        inst->staticInst->disassemble(inst->instAddr()),
                        size, q);
                return(WidthInfo(WidthClass::SimdPackingAdd,
                                 widthOp2VectorRegl(inst, q, size, 2, 3),
                                 size));
            }
            break;
        case 0x06:
            if (size_q != 0x6) {
                // CmhiDX, CmhiQX, CmgtDX, CmgtQX
                DPRINTF(WidthDecoderDecode,
                        "Neon %s inst decoded: %s. Size: %d, Q: %d.\n",
                        (u) ? "CMHI" : "CMGT",
                        inst->staticInst->disassemble(inst->instAddr()),
                        size, q);
                return(WidthInfo(WidthClass::SimdPackingAdd,
                                 widthOp2VectorRegl(inst, q, size, 2, 3),
                                 size));
            }
            break;
        case 0x07:
            if (size_q != 0x6) {
                // CmhsDX, CmhsQX, CmgeDX, CmgeQX
                DPRINTF(WidthDecoderDecode,
                        "Neon %s inst decoded: %s. Size: %d, Q: %d.\n",
                        (u) ? "CMHS" : "CMGE",
                        inst->staticInst->disassemble(inst->instAddr()),
                        size, q);
                return(WidthInfo(WidthClass::SimdPackingAdd,
                                 widthOp2VectorRegl(inst, q, size, 2, 3),
                                 size));
            }
            break;
        case 0x0c:
            if (size != 0x3) {
                // UmaxDX, UmaxQX, SmaxDX, SmaxQX
                DPRINTF(WidthDecoderDecode,
                        "Neon MAX inst decoded: %s. Size: %d, Q: %d.\n",
                        inst->staticInst->disassemble(inst->instAddr()),
                        size, q);
                return(WidthInfo(WidthClass::SimdPackingAdd,
                                 widthOp2VectorRegl(inst, q, size, 2, 3),
                                 size));
            }
            break;
        case 0x0d:
            if (size != 0x3) {
                // UminDX, UminQX, SminDX, SminQX
                DPRINTF(WidthDecoderDecode,
                        "Neon MIN inst decoded: %s. Size: %d, Q: %d.\n",
                        inst->staticInst->disassemble(inst->instAddr()),
                        size, q);
                return(WidthInfo(WidthClass::SimdPackingAdd,
                                 widthOp2VectorRegl(inst, q, size, 2, 3),
                                 size));
            }
            break;
        case 0x0e:
            if (size != 0x3) {
                // UabdDX, UabdQX, SabdDX, SabdQX
                DPRINTF(WidthDecoderDecode,
                        "Neon ABA inst decoded: %s. Size: %d, Q: %d.\n",
                        inst->staticInst->disassemble(inst->instAddr()),
                        size, q);
                return(WidthInfo(WidthClass::SimdPackingAdd,
                                 widthOp2VectorRegl(inst, q, size, 2, 3),
                                 size));
            }
            break;
        case 0x0f:
            if (size != 0x3) {
                // UabaDX, UabaQX, SabaDX, SabaQX
                DPRINTF(WidthDecoderDecode,
                        "Neon ABA inst decoded: %s. Size: %d, Q: %d.\n",
                        inst->staticInst->disassemble(inst->instAddr()),
                        size, q);
                return(WidthInfo(WidthClass::SimdPackingAdd,
                                 widthOp2VectorRegl(inst, q, size, 2, 3),
                                 size));
            }
            break;
        case 0x10:
            if (size_q != 0x6) {
                // SubDX, SubQX, AddDX, AddQX
                DPRINTF(WidthDecoderDecode,
                        "Neon %s inst decoded: %s. Size: %d, Q: %d.\n",
                        (u) ? "SUB" : "ADD",
                        inst->staticInst->disassemble(inst->instAddr()),
                        size, q);
                return(WidthInfo(WidthClass::SimdPackingAdd,
                                 widthOp2VectorRegl(inst, q, size, 2, 3),
                                 size));
            }
            break;
        case 0x11:
            if (size_q != 0x6) {
                if (u) {
                    // CmeqDX, CmeqQX
                    DPRINTF(WidthDecoderDecode,
                            "Neon CMEQ inst decoded: %s. Size: %d, Q: %d.\n",
                            inst->staticInst->disassemble(inst->instAddr()),
                            size, q);
                    return(WidthInfo(WidthClass::SimdPackingAdd,
                                     widthOp2VectorRegl(inst, q, size, 2, 3),
                                     size));
                } else {
                    // CmtstDX, CmtstQX
                    DPRINTF(WidthDecoderDecode,
                            "Neon CMTST inst decoded: %s. Size: %d, Q: %d.\n",
                            inst->staticInst->disassemble(inst->instAddr()),
                            size, q);
                    return(WidthInfo(WidthClass::SimdPackingAlu,
                                     widthOp2VectorRegl(inst, q, size, 2, 3),
                                     size));
                }
            }
            break;
        case 0x12:
            // MlsDX, MlsQX, MlaDX, MlaQX
            if (size != 0x3) {
                DPRINTF(WidthDecoderDecode,
                        "Neon %s inst decoded: %s. Size: %d, Q: %d.\n",
                        (u) ? "MLS" : "MLA",
                        inst->staticInst->disassemble(inst->instAddr()),
                        size, q);
                return(WidthInfo(WidthClass::SimdPackingMult,
                                 widthOp2VectorRegl(inst, q, size, 2, 3),
                                 size));
            }
            break;
        case 0x13:
            if (!u) {
                // MulDX, MulQX
                DPRINTF(WidthDecoderDecode,
                        "Neon MUL inst decoded: %s. Size: %d, Q: %d.\n",
                        inst->staticInst->disassemble(inst->instAddr()),
                        size, q);
                return(WidthInfo(WidthClass::SimdPackingMult,
                                 widthOp2VectorRegl(inst, q, size, 2, 3),
                                 size));
            }
            break;
        case 0x14:
            if (size != 0x3) {
                // UmaxpDX, UmaxpQX, SmaxpDX, SmaxpQX
                DPRINTF(WidthDecoderDecode,
                        "Neon MAXP inst decoded: %s. Size: %d, Q: %d.\n",
                        inst->staticInst->disassemble(inst->instAddr()),
                        size, q);
                return(WidthInfo(WidthClass::SimdPackingAdd,
                                 widthOp2VectorPair(inst, q, size, 2, 3),
                                 size));
            }
            break;
        case 0x15:
            if (size != 0x3) {
                // UminpDX, UminpQX, SminpDX, SminpQX
                DPRINTF(WidthDecoderDecode,
                        "Neon MINP inst decoded: %s. Size: %d, Q: %d.\n",
                        inst->staticInst->disassemble(inst->instAddr()),
                        size, q);
                return(WidthInfo(WidthClass::SimdPackingAdd,
                                 widthOp2VectorPair(inst, q, size, 2, 3),
                                 size));
            }
            break;
        case 0x17:
            if (u || size_q == 0x6) {
                return(WidthInfo(WidthClass::NoInfo));
            } else {
                // AddpDX, AddpQX
                DPRINTF(WidthDecoderDecode,
                        "Neon ADDP inst decoded: %s. Size: %d, Q: %d.\n",
                        inst->staticInst->disassemble(inst->instAddr()),
                        size, q);
                return(WidthInfo(WidthClass::SimdPackingAdd,
                                 widthOp2VectorPair(inst, q, size, 2, 3),
                                 size));
            }
            break;
    }

    return(WidthInfo(WidthClass::NoInfo));
}

template <class Impl>
VecWidthCode
WidthDecoder<Impl>::widthOp2VectorRegl(DynInstPtr &inst,
                                       uint8_t q, uint8_t size,
                                       uint8_t op1, uint8_t op2)
{
    VecWidthCode maskOp1, maskOp2, maskRes;

    maskOp1 = vecSrcRegWidthMask(inst, q, size, op1);
    maskOp2 = vecSrcRegWidthMask(inst, q, size, op2);

    maskRes = maskOp1.combine2OpRegl(maskOp2);
    DPRINTF(WidthDecoderWidth, "Instruction with 2 vectors operands (regular)"
            " has width mask %s (eSize=%i).\n",
            maskRes.to_string(),
            size);
    return maskRes;
}

template <class Impl>
VecWidthCode
WidthDecoder<Impl>::widthOp2VectorPair(DynInstPtr &inst,
                                       uint8_t q, uint8_t size,
                                       uint8_t op1, uint8_t op2)
{
    VecWidthCode maskOp1, maskOp2, maskRes;

    maskOp1 = vecSrcRegWidthMask(inst, q, size, op1);
    maskOp2 = vecSrcRegWidthMask(inst, q, size, op2);

    maskRes = maskOp1.combine2OpPair(maskOp2);
    DPRINTF(WidthDecoderWidth, "Instruction with 2 vectors operands (pairwise)"
            " has width mask %s (eSize=%i).\n",
            maskRes.to_string(),
            size);
    return maskRes;
}

template <class Impl>
void
WidthDecoder<Impl>::regStats()
{}

#endif // __CPU_O3_WIDTH_DECODER_IMPL_HH__
/// MPINHO 12-mar-2019 END ///
