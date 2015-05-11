/*
 * Copyright (c) 2015. Markos Horro
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Markos Horro
 *
 */

/**
 * @file
 * ScratchpadMemory declaration, based on SimpleMemory
 */

#ifndef __SCRATCHPAD_MEMORY_HH__
#define __SCRATCHPAD_MEMORY_HH__

#include <deque>

#include "base/statistics.hh"
#include "mem/abstract_mem.hh"
#include "mem/port.hh"
#include "params/ScratchpadMemory.hh"

/**
 * The scratchpad is a basic double-ported memory controller with
 * configurable size and latencies
 *
 * @sa  \ref gem5MemorySystem "gem5 Memory System"
 */
class ScratchpadMemory : public AbstractMemory
{

  private:

    /**
     * A deferred packet stores a packet along with its scheduled
     * transmission time
     */
    class DeferredPacket
    {

      public:

        const Tick tick;
        const PacketPtr pkt;

        DeferredPacket(PacketPtr _pkt, Tick _tick) : tick(_tick), pkt(_pkt)
        { }
    };

    /**
     * Definition ScratchpadSlavePort
     */
    class MemoryPort : public SlavePort
    {

      private:

        ScratchpadMemory& memory;

      public:

        MemoryPort(const std::string& _name, ScratchpadMemory& _memory);

      protected:

        Tick recvAtomic(PacketPtr pkt);

        void recvFunctional(PacketPtr pkt);

        bool recvTimingReq(PacketPtr pkt);

        void recvRespRetry();

        AddrRangeList getAddrRanges() const;

    };

    MemoryPort port;

    // ScratchpadMasterPort master_port; // connect with main memory

    /**
     * Latency from that a request is accepted until the response is
     * ready to be sent.
     */
    const Tick latency_read;

    /**
     * Latency from that a request is sent until the response is
     * received.
     */
    //const Tick latency_write;

    /**
     * Fudge factor added to the read latency
     */
    const Tick latency_read_var;

    /**
     * Fudge factor added to the write latency
     */
    //const Tick latency_write_var;

    /**
     * Detemine the writting latency
     *
     * @return the latency seen by the packet requested
     */
    //Tick getLatencyWrite() const;

    /**
     * Detemine the reading latency
     *
     * @return the latency seen by the current packet
     */
    Tick getLatencyRead() const;

    /**
     * Internal (unbounded) storage to mimic the delay caused by the
     * actual memory access. Note that this is where the packet spends
     * the memory latency.
     */
    std::deque<DeferredPacket> packetQueue;

    /**
     * Bandwidth in ticks per byte. The regulation affects the
     * acceptance rate of requests and the queueing takes place after
     * the regulation.
     */
    const double bandwidth;

    /**
     * Track the state of the memory as either idle or busy, no need
     * for an enum with only two states.
     */
    bool isBusy;

    /**
     * Remember if we have to retry an outstanding request that
     * arrived while we were busy.
     */
    bool retryReq;

    /**
     * Remember if we failed to send a response and are awaiting a
     * retry. This is only used as a check.
     */
    bool retryResp;

    /**
     * Release the memory after being busy and send a retry if a
     * request was rejected in the meanwhile.
     */
    void release();

    EventWrapper<ScratchpadMemory, &ScratchpadMemory::release> releaseEvent;

    /**
     * Dequeue a packet from our internal packet queue and move it to
     * the port where it will be sent as soon as possible.
     */
    void dequeue();

    EventWrapper<ScratchpadMemory, &ScratchpadMemory::dequeue> dequeueEvent;

    /** @todo this is a temporary workaround until the 4-phase code is
     * committed. upstream caches needs this packet until true is returned, so
     * hold onto it for deletion until a subsequent call
     */
    std::vector<PacketPtr> pendingDelete;

    /**
     * If we need to drain, keep the drain manager around until we're
     * done here.
     */
    DrainManager *drainManager;

    // All statistics that the model needs to capture
    Stats::Scalar readReqs;
    Stats::Scalar writeReqs;
    Stats::Scalar readBursts;
    Stats::Scalar writeBursts;
    Stats::Scalar bytesReadDRAM;
    Stats::Scalar bytesReadWrQ;
    Stats::Scalar bytesWritten;
    Stats::Scalar bytesReadSys;
    Stats::Scalar bytesWrittenSys;
    Stats::Scalar servicedByWrQ;
    Stats::Scalar mergedWrBursts;
    Stats::Scalar neitherReadNorWrite;
    Stats::Vector perBankRdBursts;
    Stats::Vector perBankWrBursts;
    Stats::Scalar numRdRetry;
    Stats::Scalar numWrRetry;
    Stats::Scalar totGap;
    Stats::Vector readPktSize;
    Stats::Vector writePktSize;
    Stats::Vector rdQLenPdf;
    Stats::Vector wrQLenPdf;
    Stats::Histogram bytesPerActivate;
    Stats::Histogram rdPerTurnAround;
    Stats::Histogram wrPerTurnAround;

    // Latencies summed over all requests
    Stats::Scalar totQLat;
    Stats::Scalar totMemAccLat;
    Stats::Scalar totBusLat;

    // Average latencies per request
    Stats::Formula avgQLat;
    Stats::Formula avgBusLat;
    Stats::Formula avgMemAccLat;

  public:

    void regStats();

    ScratchpadMemory(const ScratchpadMemoryParams *p);

    unsigned int drain(DrainManager *dm);

    BaseSlavePort& getSlavePort(const std::string& if_name,
                                PortID idx = InvalidPortID);

    void init();

  protected:

    Tick recvAtomic(PacketPtr pkt);

    void recvFunctional(PacketPtr pkt);

    bool recvTimingReq(PacketPtr pkt);

    void recvRespRetry();

};

#endif //__SCRATCHPAD_MEMORY_HH__

