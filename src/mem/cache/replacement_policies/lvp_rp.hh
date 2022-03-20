/**
 * Copyright (c) 2018-2020 Inria
 * All rights reserved.
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
 */

/**
 * @file
 * Declaration of a Re-Reference Interval Prediction replacement policy.
 *
 * Not-Recently Used (NRU) is an approximation of LRU that uses a single bit
 * to determine if an entry is going to be re-referenced in the near or distant
 * future.
 *
 * Re-Reference Interval Prediction (RRIP) is an extension of NRU that uses a
 * re-reference prediction value to determine if entries are going to be re-
 * used in the near future or not.
 *
 * The higher the value of the RRPV, the more distant the entry is from its
 * next access.
 *
 * Bimodal Re-Reference Interval Prediction (BRRIP) is an extension of RRIP
 * that has a probability of not inserting entries as the LRU. This probability
 * is controlled by the bimodal throtle parameter (btp).
 *
 * From the original paper, this implementation of RRIP is also called
 * Static RRIP (SRRIP), as it always inserts entries with the same RRPV.
 */

#ifndef __MEM_CACHE_REPLACEMENT_POLICIES_LVP_RP_HH__
#define __MEM_CACHE_REPLACEMENT_POLICIES_LVP_RP_HH__

#include "base/sat_counter.hh"
#include "mem/cache/replacement_policies/base.hh"

namespace gem5
{

struct LVPRPParams;

GEM5_DEPRECATED_NAMESPACE(ReplacementPolicy, replacement_policy);
namespace replacement_policy
{

class LVP : public Base
{
  protected:
    /** LVP-specific implementation of replacement data. */
    struct LVPReplData : ReplacementData
    {
        Tick lastTouchTick;
        // Stores the hashed_pc
        uint8_t hashed_pc;
        uint8_t hashed_y;

        // Counter to store the maxCpast
        uint8_t max_cpast;

        /**
         * Re-Reference Interval Prediction Value.
         * Some values have specific names (according to the paper):
         * 0 -> near-immediate re-rereference interval
         * max_RRPV-1 -> long re-rereference interval
         * max_RRPV -> distant re-rereference interval
         */
        SatCounter8 rrpv;

        /** Whether the entry is valid. */
        bool conf;

        bool validPC;

        /**
         * Default constructor. Invalidate data.
         */
        LVPReplData(const int num_bits)
            : lastTouchTick(0),hashed_pc(0), hashed_y(0), max_cpast(0), rrpv(num_bits), conf(false), validPC(true)
        {
        }
    };

    /**
     * Number of RRPV bits. An entry that saturates its RRPV has the longest
     * possible re-reference interval, that is, it is likely not to be used
     * in the near future, and is among the best eviction candidates.
     * A maximum RRPV of 1 implies in a NRU.
     */
    const unsigned numRRPVBits;



  public:
    uint8_t pteMaxC[256][256];
    bool pteConf[256][256];
    typedef LVPRPParams Params;
    LVP(const Params &p);
    ~LVP() = default;

    /**
     * Invalidate replacement data to set it as the next probable victim.
     * Set RRPV as the the most distant re-reference.
     *
     * @param replacement_data Replacement data to be invalidated.
     */
    void invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
                                                                    override;

    /**
     * Touch an entry to update its replacement data.
     *
     * @param replacement_data Replacement data to be touched.
     * @param pkt Packet Data
     * @param candidates victim data
     */
    void touch(const std::shared_ptr<ReplacementData>& replacement_data, 
        const PacketPtr pkt, const ReplacementCandidates& candidates) override;

    /**
     * Touch an entry to update its replacement data.
     *
     * @param replacement_data Replacement data to be touched.
     * @param pkt Packet Data
     */
    void touch(const std::shared_ptr<ReplacementData>& replacement_data, 
        const PacketPtr pkt) override;

    void touch(const std::shared_ptr<ReplacementData>& replacement_data) const
        override;

    /**
     * Reset replacement data. Used when an entry is inserted.
     * Set RRPV according to the insertion policy used.
     *
     * @param replacement_data Replacement data to be reset.
     * @param pkt packet data
     * @param candidates victim data
     */
    void reset(const std::shared_ptr<ReplacementData>& replacement_data, 
        const PacketPtr pkt, const ReplacementCandidates& candidates) override;

    /**
     * Reset replacement data. Used when an entry is inserted.
     * Set RRPV according to the insertion policy used.
     *
     * @param replacement_data Replacement data to be reset.
     * @param pkt packet data
     */
    void reset(const std::shared_ptr<ReplacementData>& replacement_data, 
        const PacketPtr pkt) override;

    void reset(const std::shared_ptr<ReplacementData>& replacement_data) const
        override;
    /**
     * Find replacement victim using rrpv.
     *
     * @param cands Replacement candidates, selected by indexing policy.
     * @return Replacement entry to be replaced.
     */
    ReplaceableEntry* getVictim(const ReplacementCandidates& candidates)
                                                                     override;

    /**
     * Instantiate a replacement data entry.
     *
     * @return A shared pointer to the new replacement data.
     */
    std::shared_ptr<ReplacementData> instantiateEntry() override;
};

} // namespace replacement_policy
} // namespace gem5

#endif // __MEM_CACHE_REPLACEMENT_POLICIES_LVP_RP_HH__