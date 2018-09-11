from m5.params import *
from m5.proxy import *
from MemObject import MemObject

class SimpleMemobj(MemObject):
    type = 'SimpleMemobj'
    cxx_header = "learning_gem5/simple_memobj/simple_memobj.hh"

    inst_port = SlavePort("CPU side port, receives requests")
    data_port = SlavePort("CPU side port, receives requests")
    mem_side = MasterPort("Memory side port, send requests")
