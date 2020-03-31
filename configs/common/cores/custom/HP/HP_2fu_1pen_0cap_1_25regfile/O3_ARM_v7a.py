# Copyright (c) 2012 The Regents of The University of Michigan
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Authors: Ron Dreslinski


from m5.objects import *

# The high performance core FU Pool is based on Apple's A12 Vortex
# public information on execution latency and throughput
# https://www.anandtech.com/show/13392/the-iphone-xs-xs-max-review
#-unveiling-the-silicon-secrets/3

# Simple ALU Instructions have a latency of 1
class O3_ARM_v7a_Simple_Int(FUDesc):
    opList = [ OpDesc(opClass='IntAlu', opLat=1) ]
    count = 6

# Complex ALU instructions have a variable latencies
class O3_ARM_v7a_Complex_Int(FUDesc):
    opList = [ OpDesc(opClass='IntMult', opLat=4, pipelined=True),
               OpDesc(opClass='IntDiv', opLat=8, pipelined=False),
               OpDesc(opClass='IprAccess', opLat=3, pipelined=True) ]
    count = 2

# Floating point instructions
class O3_ARM_v7a_FP(FUDesc):
    opList = [ OpDesc(opClass='FloatAdd', opLat=3),
               OpDesc(opClass='FloatCmp', opLat=3),
               OpDesc(opClass='FloatCvt', opLat=3),
               OpDesc(opClass='FloatDiv', opLat=9, pipelined=False),
               OpDesc(opClass='FloatSqrt', opLat=33, pipelined=False),
               OpDesc(opClass='FloatMult', opLat=4),
               OpDesc(opClass='FloatMultAcc', opLat=4),
               OpDesc(opClass='FloatMisc', opLat=3) ]
    count = 2
    widthCap = 128
    floatp = True


# SIMD instructions
class O3_ARM_v7a_AdvSimd(FUDesc):
    opList = [ OpDesc(opClass='SimdAdd', opLat=3),
               OpDesc(opClass='SimdAddAcc', opLat=3),
               OpDesc(opClass='SimdAlu', opLat=3),
               OpDesc(opClass='SimdCmp', opLat=3),
               OpDesc(opClass='SimdCvt', opLat=3),
               OpDesc(opClass='SimdMisc', opLat=3),
               OpDesc(opClass='SimdMult',opLat=4),
               OpDesc(opClass='SimdMultAcc',opLat=4),
               OpDesc(opClass='SimdShift',opLat=3),
               OpDesc(opClass='SimdShiftAcc', opLat=3),
               OpDesc(opClass='SimdSqrt', opLat=10),
               OpDesc(opClass='SimdFloatAdd',opLat=4),
               OpDesc(opClass='SimdFloatAlu',opLat=4),
               OpDesc(opClass='SimdFloatCmp', opLat=4),
               OpDesc(opClass='SimdFloatCvt', opLat=4),
               OpDesc(opClass='SimdFloatDiv', opLat=4),
               OpDesc(opClass='SimdFloatMisc', opLat=4),
               OpDesc(opClass='SimdFloatMult', opLat=5),
               OpDesc(opClass='SimdFloatMultAcc',opLat=5),
               OpDesc(opClass='SimdFloatSqrt', opLat=10) ]
    count = 2
    fuseCap = 0
    widthCap = 128
    simd = True

# Load/Store Units
class O3_ARM_v7a_Load(FUDesc):
    opList = [ OpDesc(opClass='MemRead', opLat=2),
               OpDesc(opClass='FloatMemRead', opLat=2) ]
    count = 2
    widthCap = 128

class O3_ARM_v7a_Store(FUDesc):
    opList = [ OpDesc(opClass='MemWrite', opLat=2),
               OpDesc(opClass='FloatMemWrite', opLat=2) ]
    count = 2
    widthCap = 128

# Functional Units for this CPU
class O3_ARM_v7a_FUP(FUPool):
    FUList = [O3_ARM_v7a_Simple_Int(),
              O3_ARM_v7a_Complex_Int(),
              O3_ARM_v7a_Load(),
              O3_ARM_v7a_Store(),
              O3_ARM_v7a_FP(),
              O3_ARM_v7a_AdvSimd()]

# Bi-Mode Branch Predictor
class O3_ARM_v7a_BP(BiModeBP):
    globalPredictorSize = 8192
    globalCtrBits = 2
    choicePredictorSize = 8192
    choiceCtrBits = 2
    BTBEntries = 4096
    BTBTagSize = 16
    RASSize = 16
    instShiftAmt = 2

class O3_ARM_v7a_3(DerivO3CPU):
    LQEntries = 68
    SQEntries = 72
    LSQDepCheckShift = 0
    LFSTSize = 1024
    SSITSize = 1024
    decodeToFetchDelay = 1
    renameToFetchDelay = 1
    iewToFetchDelay = 1
    commitToFetchDelay = 1
    renameToDecodeDelay = 1
    iewToDecodeDelay = 1
    commitToDecodeDelay = 1
    iewToRenameDelay = 1
    commitToRenameDelay = 1
    commitToIEWDelay = 1
    fetchWidth = 8
    fetchBufferSize = 32
    fetchToDecodeDelay = 3
    decodeWidth = 8
    decodeToRenameDelay = 2
    renameWidth = 8
    renameToIEWDelay = 1
    issueToExecuteDelay = 1
    dispatchWidth = 12
    issueWidth = 12
    wbWidth = 12
    fuPool = O3_ARM_v7a_FUP()
    iewToCommitDelay = 1
    renameToROBDelay = 1
    commitWidth = 12
    squashWidth = 12
    trapLatency = 13
    backComSize = 5
    forwardComSize = 5
    numPhysIntRegs = 480
    numPhysFloatRegs = 480
    numPhysVecRegs = 480
    numIQEntries = 180
    numROBEntries = 192

    switched_out = False
    branchPred = O3_ARM_v7a_BP()

# Instruction Cache
class O3_ARM_v7a_ICache(Cache):
    tag_latency = 1
    data_latency = 1
    response_latency = 1
    mshrs = 8
    tgts_per_mshr = 8
    size = '64kB'
    assoc = 4
    is_read_only = True
    # Writeback clean lines as well
    writeback_clean = True

# Data Cache
class O3_ARM_v7a_DCache(Cache):
    tag_latency = 2
    data_latency = 2
    response_latency = 1
    mshrs = 20
    tgts_per_mshr = 16
    size = '64kB'
    assoc = 4
    write_buffers = 24
    # Consider the L2 a victim cache also for clean lines
    writeback_clean = True

# TLB Cache
# Use a cache as a L2 TLB
class O3_ARM_v7aWalkCache(Cache):
    tag_latency = 4
    data_latency = 4
    response_latency = 4
    mshrs = 6
    tgts_per_mshr = 8
    size = '1kB'
    assoc = 8
    write_buffers = 16
    is_read_only = True
    # Writeback clean lines as well
    writeback_clean = True

# L2 Cache
class O3_ARM_v7aL2(Cache):
    tag_latency = 9
    data_latency = 9
    response_latency = 5
    mshrs = 46
    tgts_per_mshr = 16
    size = '256kB'
    assoc = 8
    write_buffers = 24
    prefetch_on_access = True
    clusivity = 'mostly_incl'
    # Simple stride prefetcher
    prefetcher = StridePrefetcher(degree=8, latency = 1)
    tags = BaseSetAssoc()
    repl_policy = RandomRP()