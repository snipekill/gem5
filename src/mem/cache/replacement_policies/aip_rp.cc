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

#include "mem/cache/replacement_policies/aip_rp.hh"

#include <cassert>
#include <memory>

#include "debug/CacheRepl.hh"
#include "base/trace.hh"

#include "base/logging.hh" // For fatal_if
#include "base/random.hh"
#include "params/AIPRP.hh"

namespace gem5
{

GEM5_DEPRECATED_NAMESPACE(ReplacementPolicy, replacement_policy);
namespace replacement_policy
{

AIP::AIP(const Params &p)
  : Base(p), numRRPVBits(p.num_bits)
{
    for(unsigned int i=0;i<256;i++){
        for(unsigned int j=0;j<256;j++){
            pteMaxC[i][j] = 0;
            pteConf[i][j] = false;
        }
    }
    fatal_if(numRRPVBits <= 0, "There should be at least one bit per RRPV.\n");
    fatal_if(numRRPVBits > 8, "There should be at max 8 bits per RRPV.\n");
}

void
AIP::invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
{
    try
    {
        /* code */
        std::shared_ptr<AIPReplData> casted_replacement_data =
            std::static_pointer_cast<AIPReplData>(replacement_data);


            // Reset last touch timestamp
        std::static_pointer_cast<AIPReplData>(
            replacement_data)->lastTouchTick = Tick(0);

        if(!casted_replacement_data->validPC){
            return;
        }
        
        uint8_t maxCStored = pteMaxC[casted_replacement_data->hashed_pc][casted_replacement_data->hashed_y];

        pteMaxC[casted_replacement_data->hashed_pc][casted_replacement_data->hashed_y] = casted_replacement_data->max_cpresent;
        if(maxCStored == casted_replacement_data->max_cpast){
            pteConf[casted_replacement_data->hashed_pc][casted_replacement_data->hashed_y] = true;
        }
        else
            pteConf[casted_replacement_data->hashed_pc][casted_replacement_data->hashed_y] = false;
    }
    catch(const std::exception& e)
    {
        DPRINTF(CacheRepl, "Error Thrown Idiot(Invalidate): %s\n", e.what());
    }
    
}

void
AIP::touch(const std::shared_ptr<ReplacementData>& replacement_data, const PacketPtr pkt, const ReplacementCandidates& candidates)
{
    // update each line counter

        // Visit all candidates to find victim
    try
    {
        /* code */
        for (const auto& candidate : candidates) {
            std::shared_ptr<AIPReplData> candidate_repl_data =
                std::static_pointer_cast<AIPReplData>(
                    candidate->replacementData);

            candidate_repl_data->rrpv += 1;
            
        }

        //reset rrpv counter to 0 after updating max C counter

        std::shared_ptr<AIPReplData> casted_replacement_data =
            std::static_pointer_cast<AIPReplData>(replacement_data);

        if (casted_replacement_data->rrpv > casted_replacement_data->max_cpresent){
            casted_replacement_data->max_cpresent = casted_replacement_data->rrpv;
        }

        casted_replacement_data->rrpv.reset();
        // Update last touch timestamp
        std::static_pointer_cast<AIPReplData>(
            replacement_data)->lastTouchTick = curTick();
    }
    catch(const std::exception& e)
    {
        DPRINTF(CacheRepl, "Error Thrown Idiot(Touch Function): %s\n", e.what());
    }
    
    
}


void
AIP::touch(const std::shared_ptr<ReplacementData>& replacement_data, const PacketPtr pkt)
{
    panic("Cant train AIP's predictor without access information.");
}

void
AIP::touch(const std::shared_ptr<ReplacementData>& replacement_data)
    const
{
    panic("Cant train AIP's predictor without access information.");
}


void
AIP::reset(const std::shared_ptr<ReplacementData>& replacement_data, const PacketPtr pkt, const ReplacementCandidates& candidates)
{
    try
    {
        /* code */
        std::shared_ptr<AIPReplData> casted_replacement_data =
            std::static_pointer_cast<AIPReplData>(replacement_data);

            // Set last touch timestamp
        casted_replacement_data->lastTouchTick = curTick();

        // Reset RRPV
        // Replacement data is inserted as "long re-reference" if lower than btp,
        // "distant re-reference" otherwise
        
        // check if instruction has PC
        if (!pkt->req->hasPC()){
            casted_replacement_data->validPC = false;
            return;
        }
        uint64_t PC = pkt->req->getPC();
        uint8_t hashedPC = (uint8_t)PC;
        for(unsigned int i=1;i<8;i++){
            PC = PC >> 8;
            hashedPC = hashedPC ^ PC;
        }
        uint64_t Y = pkt->getAddr();
        uint8_t hashedY = (uint8_t)Y;
        for(unsigned int i=1;i<8;i++){
            Y = Y >> 8;
            hashedY = hashedY ^ Y;
        }
        casted_replacement_data->hashed_pc = hashedPC;
        casted_replacement_data->hashed_y = hashedY;
        casted_replacement_data->rrpv.reset();

        casted_replacement_data->max_cpresent = 0;
        casted_replacement_data->max_cpast = pteMaxC[hashedPC][hashedY];
        casted_replacement_data->conf = pteConf[hashedPC][hashedY];
    }
    catch(const std::exception& e)
    {
        DPRINTF(CacheRepl, "Error Thrown Idiot (RESET Function): %s\n", e.what());
    }
    
}


void
AIP::reset(const std::shared_ptr<ReplacementData>& replacement_data, const PacketPtr pkt)
{
    panic("Cant train AIP's predictor without access information.");
}

void
AIP::reset(const std::shared_ptr<ReplacementData>& replacement_data)
    const
{
    panic("Cant train AIP's predictor without access information.");
}

ReplaceableEntry*
AIP::getVictim(const ReplacementCandidates& candidates)
{
    try
    {
        /* code */
        // There must be at least one replacement candidate
        assert(candidates.size() > 0);

        // Use first candidate as dummy victim
        ReplaceableEntry* LRUvictim = candidates[0];

        unsigned int expired_indices[candidates.size()+1];
        unsigned int expired_entries = 0;

        for(unsigned int i = 0; i<candidates.size();i++){
            ReplaceableEntry* candidate = candidates[i];
            std::shared_ptr<AIPReplData> candidate_repl_data =
                std::static_pointer_cast<AIPReplData>(
                    candidate->replacementData);
            candidate_repl_data->rrpv += 1;

            if (candidate_repl_data->lastTouchTick <
                    std::static_pointer_cast<AIPReplData>(
                        LRUvictim->replacementData)->lastTouchTick) {
                LRUvictim = candidate;
            }

            if(candidate_repl_data->rrpv > candidate_repl_data->max_cpresent && 
                candidate_repl_data->rrpv > candidate_repl_data->max_cpast &&
                    candidate_repl_data->conf == 1){
                    expired_indices[expired_entries++] = i; 
            }

        }

        if(std::static_pointer_cast<AIPReplData>(
                            LRUvictim->replacementData)->lastTouchTick == 0){
                                return LRUvictim;
        }

        ReplaceableEntry* finalVictim = LRUvictim;

        if(expired_entries > 0){
            unsigned int random_index = random_mt.random<unsigned>(0, expired_entries-1);
            finalVictim = candidates[expired_indices[random_index]];
        }

        std::shared_ptr<AIPReplData> final_victim_repl_data =
            std::static_pointer_cast<AIPReplData>(
                finalVictim->replacementData);

        if(!final_victim_repl_data->validPC){
            return finalVictim;
        }

        uint8_t maxCStored = pteMaxC[final_victim_repl_data->hashed_pc][final_victim_repl_data->hashed_y];

        pteMaxC[final_victim_repl_data->hashed_pc][final_victim_repl_data->hashed_y] = final_victim_repl_data->max_cpresent;
        if(maxCStored == final_victim_repl_data->max_cpast){
            pteConf[final_victim_repl_data->hashed_pc][final_victim_repl_data->hashed_y] = true;
        }
        else
            pteConf[final_victim_repl_data->hashed_pc][final_victim_repl_data->hashed_y] = false;

        return finalVictim;
    }
    catch(const std::exception& e)
    {
        DPRINTF(CacheRepl, "Error Thrown Idiot (getVictim Function): %s\n", e.what());
        assert(candidates.size() > 0);
        return candidates[0];
    }
    
}

std::shared_ptr<ReplacementData>
AIP::instantiateEntry()
{
    return std::shared_ptr<ReplacementData>(new AIPReplData(numRRPVBits));
}

} // namespace replacement_policy
} // namespace gem5
