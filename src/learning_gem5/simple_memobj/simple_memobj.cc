#include "learning_gem5/simple_memobj/simple_memobj.hh"

#include "debug/SimpleMemobj.hh"

SimpleMemobj::SimpleMemobj(SimpleMemobjParams *params) :
    MemObject(params),
    instPort(params->name + ".inst_port", this),
    dataPort(params->name + ".data_port", this),
    memPort(params->name + ".mem_port", this)
{
}

BaseMasterPort&
SimpleMemobj::getMasterPort(const std::string& if_name, PortID idx)
{
    if (if_name == "mem_side") {
        return memPort;
    } else {
        return MemObject::getMasterPort(if_name, idx);
    }
}

BaseSlavePort&
SimpleMemobj::getSlavePort(const std::string& if_name, PortID idx)
{
    if (if_name == "inst_port") {
        return instPort;
    } else if (if_name == "data_port") {
        return dataPort;
    } else {
        return MemObject::getSlavePort(if_name, idx);
    }
}

AddrRangeList
SimpleMemobj::CPUSidePort::getAddrRanges() const
{
    return owner->getAddrRanges();
}

void
SimpleMemobj::CPUSidePort::recvFunctional(PacketPtr pkt)
{
    return owner->handleFunctional(pkt);
}

bool
SimpleMemobj::CPUSidePort::recvTimingReq(PacketPtr pkt)
{
    if (!owner->handleRequest(pkt)) {
        needRetry = true;
        return false;
    } else {
        return true;
    }
}

void
SimpleMemobj::CPUSidePort::sendPacket(PacketPtr pkt)
{
    panic_if(blockedPacket != nullptr, "Should never try to send if blocked!");

    if (!sendTimingResp(pkt)) {
        blockedPacket = pkt;
    }
}

void
SimpleMemobj::CPUSidePort::recvRespRetry()
{
    assert(blockedPacket != nullptr);

    PacketPtr pkt = blockedPacket;
    blockedPacket = nullptr;

    sendPacket(pkt);
}

void
SimpleMemobj::CPUSidePort::trySendRetry()
{
    if (needRetry && blockedPacket == nullptr) {
        needRetry = false;
        DPRINTF(SimpleMemobj, "Sending retry req for %d\n", id);
        sendRetryReq();
    }
}

void
SimpleMemobj::MemSidePort::recvRangeChange()
{
    owner->sendRangeChange();
}

void
SimpleMemobj::MemSidePort::sendPacket(PacketPtr pkt)
{
    panic_if(blockedPacket != nullptr, "Should never try to send if blocked!");
    if (!sendTimingReq(pkt)) {
        blockedPacket = pkt;
    }
}

void
SimpleMemobj::MemSidePort::recvReqRetry()
{
    assert(blockedPacket != nullptr);

    PacketPtr pkt = blockedPacket;
    blockedPacket = nullptr;

    sendPacket(pkt);
}

bool
SimpleMemobj::MemSidePort::recvTimingResp(PacketPtr pkt)
{
    return owner->handleResponse(pkt);
}

void
SimpleMemobj::handleFunctional(PacketPtr pkt)
{
    memPort.sendFunctional(pkt);
}

AddrRangeList
SimpleMemobj::getAddrRanges() const
{
    DPRINTF(SimpleMemobj, "Sending new ranges\n");
    return memPort.getAddrRanges();
}

void
SimpleMemobj::sendRangeChange()
{
    instPort.sendRangeChange();
    dataPort.sendRangeChange();
}

bool
SimpleMemobj::handleRequest(PacketPtr pkt)
{
    if (blocked) {
        return false;
    }
    DPRINTF(SimpleMemobj, "Got request for addr %#x\n", pkt->getAddr());
    blocked = true;
    memPort.sendPacket(pkt);
    return true;
}

bool
SimpleMemobj::handleResponse(PacketPtr pkt)
{
    assert(blocked);
    DPRINTF(SimpleMemobj, "Got response for addr %#x\n", pkt->getAddr());

    blocked = false;

    // Simply forward to the memory port
    if (pkt->req->isInstFetch()) {
        instPort.sendPacket(pkt);
    } else {
        dataPort.sendPacket(pkt);
    }

    instPort.trySendRetry();
    dataPort.trySendRetry();

    return true;
}

SimpleMemobj*
SimpleMemobjParams::create()
{
    return new SimpleMemobj(this);
}
