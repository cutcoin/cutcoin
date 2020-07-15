// Copyright (c) 2018-2020, CUT coin
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
#include "ringct/rctOps.h"

#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>

namespace cryptonote {

struct tx_source_entry
{
  typedef std::pair<uint64_t, rct::ctkey> output_entry;

  std::vector<output_entry> outputs;  // index + key + optional ringct commitment
  size_t real_output;                 // index in outputs vector of real output_entry
  crypto::public_key real_out_tx_key; // incoming real tx public key
  std::vector<crypto::public_key> real_out_additional_tx_keys; // incoming real tx additional public keys
  size_t real_output_in_tx_index;     // index in transaction outputs vector
  uint64_t amount;                    // money
  bool rct;                           // true if the output is rct
  rct::key mask;                      // ringct amount mask
  rct::multisig_kLRki multisig_kLRki; // multisig info
  TokenId token_id;                   // tx source token ID

  void push_output(uint64_t idx, const crypto::public_key &k, uint64_t amount)
  {
    outputs.emplace_back(std::make_pair(idx, rct::ctkey({rct::pk2rct(k), rct::zeroCommit(amount)})));
  }

  void push_output(uint64_t idx, const crypto::public_key &k, uint64_t amount, TokenId tid)
  {
    outputs.emplace_back(std::make_pair(idx, rct::ctkey({rct::pk2rct(k), rct::gp_zeroCommit(amount, rct::tokenIdToPoint(tid))})));
  }

  BEGIN_SERIALIZE_OBJECT()
    FIELD(outputs)
    FIELD(real_output)
    FIELD(real_out_tx_key)
    FIELD(real_out_additional_tx_keys)
    FIELD(real_output_in_tx_index)
    FIELD(amount)
    FIELD(rct)
    FIELD(mask)
    FIELD(multisig_kLRki)

    if (real_output >= outputs.size())
      return false;
  END_SERIALIZE()
};

using tx_sources = std::vector<cryptonote::tx_source_entry>;

struct tx_destination_entry
{
  TokenId                token_id;      // token id
  uint64_t               amount;        // money
  account_public_address addr;          // destination address
  bool                   is_subaddress; // show if the destination is subaddress

  tx_destination_entry()
    : amount(0)
    , addr(AUTO_VAL_INIT(addr))
    , is_subaddress(false)
    , token_id(CUTCOIN_ID)
    { }
  // Create this object. Default constructor.

  tx_destination_entry(TokenId id, uint64_t a, const account_public_address &ad, bool is_subaddress)
    : amount(a)
    , addr(ad)
    , is_subaddress(is_subaddress)
    , token_id(id) { }
  // Create this object.

  BEGIN_SERIALIZE_OBJECT()
    VARINT_FIELD(amount)
    FIELD(addr)
    FIELD(is_subaddress)
    VARINT_FIELD(token_id)
  END_SERIALIZE()
};

void classify_addresses(const std::vector<tx_destination_entry>       &destinations,
                        const boost::optional<account_public_address> &change_addr,
                        size_t                                        &num_stdaddresses,
                        size_t                                        &num_subaddresses,
                        account_public_address                        &single_dest_subaddress);

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

  //---------------------------------------------------------------
  crypto::public_key get_destination_view_key_pub(const std::vector<tx_destination_entry>       &destinations,
                                                  const boost::optional<account_public_address> &change_addr);

  bool construct_tx(const account_keys                            &sender_account_keys,
                    tx_sources                                    &sources,
                    const std::vector<tx_destination_entry>       &destinations,
                    const boost::optional<account_public_address> &change_addr,
                    std::vector<uint8_t>                           extra,
                    transaction                                   &tx,
                    uint64_t                                       unlock_time);

  bool construct_tx_with_tx_key(const account_keys                                             &sender_account_keys,
                                const std::unordered_map<crypto::public_key, subaddress_index> &subaddresses,
                                tx_sources                                                     &sources,
                                std::vector<tx_destination_entry>                              &destinations,
                                const boost::optional<account_public_address>                  &change_addr,
                                std::vector<uint8_t>                                            extra,
                                transaction                                                    &tx,
                                uint64_t                                                        unlock_time,
                                const crypto::secret_key                                       &tx_key,
                                const std::vector<crypto::secret_key>                          &additional_tx_keys,
                                bool                                                            use_tokens,
                                bool                                                            rct = false,
                                rct::RangeProofType                                             range_proof_type = rct::RangeProofBorromean,
                                rct::multisig_out                                              *msout = NULL,
                                bool                                                            shuffle_outs = true,
                                bool                                                            tgtx = false);

  bool construct_tx_and_get_tx_key(const account_keys                                             &sender_account_keys,
                                   const std::unordered_map<crypto::public_key, subaddress_index> &subaddresses,
                                   tx_sources                                                     &sources,
                                   std::vector<tx_destination_entry>                              &destinations,
                                   const boost::optional<account_public_address>                  &change_addr,
                                   std::vector<uint8_t>                                            extra,
                                   transaction                                                    &tx,
                                   uint64_t                                                        unlock_time,
                                   crypto::secret_key                                             &tx_key,
                                   std::vector<crypto::secret_key>                                &additional_tx_keys,
                                   bool                                                            use_tokens,
                                   bool                                                            rct = false,
                                   rct::RangeProofType                                             range_proof_type = rct::RangeProofBorromean,
                                   rct::multisig_out                                              *msout = NULL,
                                   bool                                                            tgtx = false);

  bool construct_token_tx_with_tx_key(const account_keys                                             &sender_account_keys,
                                      const std::unordered_map<crypto::public_key, subaddress_index> &cutcoin_subaddresses,
                                      tx_sources                                                     &cutcoin_sources,
                                      std::vector<tx_destination_entry>                              &destinations,
                                      const boost::optional<account_public_address>                  &change_addr,
                                      std::vector<uint8_t>                                            extra,
                                      transaction                                                    &tx,
                                      uint64_t                                                        unlock_time,
                                      const crypto::secret_key                                       &tx_key,
                                      const std::vector<crypto::secret_key>                          &additional_tx_keys,
                                      bool                                                            use_tokens,
                                      bool                                                            rct = false,
                                      bool                                                            shuffle_outs = true);

  bool construct_token_tx_and_get_tx_key(const account_keys                                             &sender_account_keys,
                                         const std::unordered_map<crypto::public_key, subaddress_index> &subaddresses,
                                         tx_sources                                                     &sources,
                                         std::vector<tx_destination_entry>                              &destinations,
                                         const boost::optional<account_public_address>                  &change_addr,
                                         std::vector<uint8_t>                                            extra,
                                         transaction                                                    &tx,
                                         uint64_t                                                        unlock_time,
                                         crypto::secret_key                                             &tx_key,
                                         std::vector<crypto::secret_key>                                &additional_tx_keys);

void decompose_token_supply(TokenUnit token_supply, std::vector<uint64_t> &out_amounts);

bool generate_genesis_block(block& bl, std::string const & genesis_tx, uint32_t nonce);

} // namespace cryptonote

BOOST_CLASS_VERSION(cryptonote::tx_source_entry, 1)
BOOST_CLASS_VERSION(cryptonote::tx_destination_entry, 2)

namespace boost
{
  namespace serialization
  {
    template <class Archive>
    inline void serialize(Archive &a, cryptonote::tx_source_entry &x, const boost::serialization::version_type ver)
    {
      a & x.outputs;
      a & x.real_output;
      a & x.real_out_tx_key;
      a & x.real_output_in_tx_index;
      a & x.amount;
      a & x.rct;
      a & x.mask;
      if (ver < 1)
        return;
      a & x.multisig_kLRki;
      a & x.real_out_additional_tx_keys;
    }

    template <class Archive>
    inline void serialize(Archive& a, cryptonote::tx_destination_entry& x, const boost::serialization::version_type ver)
    {
      a & x.amount;
      a & x.addr;
      if (ver < 1) {
        return;
      }
      a & x.is_subaddress;
      if (ver < 2) {
        x.token_id = cryptonote::CUTCOIN_ID;
        return;
      }
      a & x.token_id;
    }
  }

}  // namespace boost
