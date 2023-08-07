// Copyright (c) 2018-2022, CUT coin
// Copyright (c) 2014-2018, The Monero Project
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Parts of this file are originally copyright (c) 2012-2013 The Cryptonote developers

#pragma once

#include "cryptonote_basic/cryptonote_format_utils.h"
#include "cryptonote_basic/token.h"
#include "cryptonote_core/tx_construction_context.h"
#include "ringct/rctOps.h"
#include "tx_destination_entry.h"
#include "tx_source_entry.h"

#include <vector>

namespace cryptonote {

void classify_addresses(const std::vector<tx_destination_entry> &destinations,
                        const std::vector<tx_destination_entry> &change_destinations,
                        size_t                                  &num_stdaddresses,
                        size_t                                  &num_subaddresses,
                        account_public_address                  &single_dest_subaddress);

bool construct_miner_tx(size_t                        height,
                        size_t                        median_weight,
                        uint64_t                      already_generated_coins,
                        size_t                        current_block_weight,
                        uint64_t                      fee,
                        const account_public_address &miner_address,
                        transaction                  &tx,
                        const blobdata               &extra_nonce = blobdata(),
                        size_t                        max_outs = 999,
                        uint8_t                       hard_fork_version = 1);

crypto::public_key get_destination_view_key_pub(const std::vector<tx_destination_entry>       &destinations,
                                                const boost::optional<account_public_address> &change_addr);

bool construct_tx(const account_keys                            &sender_account_keys,
                  TxSources                                     &sources,
                  const std::vector<tx_destination_entry>       &destinations,
                  const boost::optional<account_public_address> &change_addr,
                  std::vector<uint8_t>                           extra,
                  transaction                                   &tx,
                  uint64_t                                       unlock_time);

bool construct_tx_with_tx_key(const TxConstructionContext &context, transaction &tx);

bool construct_tx_and_get_tx_key(TxConstructionContext &context, transaction &tx);

bool construct_unified_tx_with_tx_key(const TxConstructionContext &context, transaction &tx);

bool construct_unified_tx_and_get_tx_key(TxConstructionContext &context, transaction &tx);

bool construct_token_gen_tx_with_tx_key(TxConstructionContext &context, transaction &tx);

bool construct_token_gen_tx_and_get_tx_key(TxConstructionContext &context, transaction &tx);

void decompose_token_supply(const Amount        &token_supply,
                            std::vector<Amount> &out_amounts,
                            size_t               amounts_count = TOKEN_GENESIS_OUTPUTS);

bool generate_genesis_block(block& bl, std::string const & genesis_tx, uint32_t nonce);

} // namespace cryptonote
