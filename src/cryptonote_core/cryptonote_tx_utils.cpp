// Copyright (c) 2018-2021, CUT coin
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

#include "include_base_utils.h"
#include "string_tools.h"
using namespace epee;

#include "common/apply_permutation.h"
#include "cryptonote_tx_utils.h"
#include "cryptonote_config.h"
#include "cryptonote_basic/cryptonote_basic.h"
#include "cryptonote_basic/miner.h"
#include "crypto/crypto.h"
#include "crypto/hash.h"
#include "multisig/multisig.h"
#include "ringct/rctSigs.h"


#include <algorithm>
#include <iterator>
#include <random>
#include <unordered_map>
#include <unordered_set>

using namespace crypto;

namespace cryptonote {

void classify_addresses(const std::vector<tx_destination_entry>       &destinations,
                        const boost::optional<account_public_address> &change_addr,
                        size_t                                        &num_stdaddresses,
                        size_t                                        &num_subaddresses,
                        account_public_address                        &single_dest_subaddress)
{
  num_stdaddresses = 0;
  num_subaddresses = 0;
  std::unordered_set<account_public_address> unique_dst_addresses;
  for(const tx_destination_entry& dst_entr: destinations)
  {
    if (change_addr && dst_entr.addr == change_addr)
      continue;
    if (unique_dst_addresses.count(dst_entr.addr) == 0)
    {
      unique_dst_addresses.insert(dst_entr.addr);
      if (dst_entr.is_subaddress)
      {
        ++num_subaddresses;
        single_dest_subaddress = dst_entr.addr;
      }
      else
      {
        ++num_stdaddresses;
      }
    }
  }
  LOG_PRINT_L2("destinations include " << num_stdaddresses << " standard addresses and " << num_subaddresses << " subaddresses");
}

bool construct_miner_tx(size_t                        height,
                        size_t                        median_weight,
                        uint64_t                      already_generated_coins,
                        size_t                        current_block_weight,
                        uint64_t                      fee,
                        const account_public_address &miner_address,
                        transaction                  &tx,
                        const blobdata               &extra_nonce,
                        size_t                        max_outs,
                        uint8_t                       hard_fork_version) {
  tx.vin.clear();
  tx.vout.clear();
  tx.extra.clear();

  keypair txkey = keypair::generate(hw::get_device("default"));
  add_tx_pub_key_to_extra(tx, txkey.pub);
  if(!extra_nonce.empty())
    if(!add_extra_nonce_to_tx_extra(tx.extra, extra_nonce))
      return false;

  txin_gen in;
  in.height = height;

  uint64_t block_reward;
  if(!get_block_reward(median_weight, current_block_weight, already_generated_coins, block_reward, hard_fork_version))
  {
    LOG_PRINT_L0("Block is too big");
    return false;
  }

#if defined(DEBUG_CREATE_BLOCK_TEMPLATE)
  LOG_PRINT_L1("Creating block template: reward " << block_reward <<
    ", fee " << fee);
#endif
  block_reward += fee;

  // from hard fork 2, we cut out the low significant digits. This makes the tx smaller, and
  // keeps the paid amount almost the same. The unpaid remainder gets pushed back to the
  // emission schedule
  // from hard fork 4, we use a single "dusty" output. This makes the tx even smaller,
  // and avoids the quantization. These outputs will be added as rct outputs with identity
  // masks, to they can be used as rct inputs.
  if (hard_fork_version >= 2 && hard_fork_version < 4) {
    block_reward = block_reward - block_reward % ::config::BASE_REWARD_CLAMP_THRESHOLD;
  }

  std::vector<uint64_t> out_amounts;
  decompose_amount_into_digits(block_reward, hard_fork_version >= 2 ? 0 : ::config::DEFAULT_DUST_THRESHOLD,
    [&out_amounts](uint64_t a_chunk) { out_amounts.push_back(a_chunk); },
    [&out_amounts](uint64_t a_dust) { out_amounts.push_back(a_dust); });

  CHECK_AND_ASSERT_MES(1 <= max_outs, false, "max_out must be non-zero");
  if (height == 0 || hard_fork_version >= 4)
  {
    // the genesis block was not decomposed, for unknown reasons
    while (max_outs < out_amounts.size())
    {
      //out_amounts[out_amounts.size() - 2] += out_amounts.back();
      //out_amounts.resize(out_amounts.size() - 1);
      out_amounts[1] += out_amounts[0];
      for (size_t n = 1; n < out_amounts.size(); ++n)
        out_amounts[n - 1] = out_amounts[n];
      out_amounts.pop_back();
    }
  }
  else
  {
    CHECK_AND_ASSERT_MES(max_outs >= out_amounts.size(), false, "max_out exceeded");
  }

  uint64_t summary_amounts = 0;
  for (size_t no = 0; no < out_amounts.size(); no++)
  {
    crypto::key_derivation derivation = AUTO_VAL_INIT(derivation);;
    crypto::public_key out_eph_public_key = AUTO_VAL_INIT(out_eph_public_key);
    bool r = crypto::generate_key_derivation(miner_address.m_view_public_key, txkey.sec, derivation);
    CHECK_AND_ASSERT_MES(r, false, "while creating outs: failed to generate_key_derivation(" << miner_address.m_view_public_key << ", " << txkey.sec << ")");

    r = crypto::derive_public_key(derivation, no, miner_address.m_spend_public_key, out_eph_public_key);
    CHECK_AND_ASSERT_MES(r, false, "while creating outs: failed to derive_public_key(" << derivation << ", " << no << ", "<< miner_address.m_spend_public_key << ")");

    txout_to_key tk;
    tk.key = out_eph_public_key;

    tx_out out;
    summary_amounts += out.amount = out_amounts[no];
    out.target = tk;
    tx.vout.push_back(out);
  }

  CHECK_AND_ASSERT_MES(summary_amounts == block_reward, false, "Failed to construct miner tx, summary_amounts = " << summary_amounts << " not equal block_reward = " << block_reward);

  if (hard_fork_version >= 11) {
    tx.version = TxVersion::tokens;
  } else if (hard_fork_version >= 4)
    tx.version = TxVersion::ring_signatures;
  else
    tx.version = TxVersion::plain;

  //lock
  tx.unlock_time = height + CRYPTONOTE_MINED_MONEY_UNLOCK_WINDOW;
  tx.vin.push_back(in);

  tx.invalidate_hashes();

  //LOG_PRINT("MINER_TX generated ok, block_reward=" << print_money(block_reward) << "("  << print_money(block_reward - fee) << "+" << print_money(fee)
  //  << "), current_block_size=" << current_block_size << ", already_generated_coins=" << already_generated_coins << ", tx_id=" << get_transaction_hash(tx), LOG_LEVEL_2);
  return true;
}

crypto::public_key get_destination_view_key_pub(const std::vector<tx_destination_entry>       &destinations,
                                                const boost::optional<account_public_address> &change_addr)
{
  account_public_address addr = {crypto::NullKey::p(), crypto::NullKey::p()};
  size_t count = 0;
  for (const auto &i : destinations)
  {
    if (i.amount == 0)
      continue;
    if (change_addr && i.addr == *change_addr)
      continue;
    if (i.addr == addr)
      continue;
    if (count > 0)
      return crypto::NullKey::p();
    addr = i.addr;
    ++count;
  }
  if (count == 0 && change_addr)
    return change_addr->m_view_public_key;
  return addr.m_view_public_key;
}

bool construct_tx_with_tx_key(const TxConstructionContext &context, transaction &tx)
{
  hw::device &hwdev = context.d_sender_account_keys.get_device();

  if (context.d_sources.empty()) {
    LOG_ERROR("Empty sources");
    return false;
  }

  std::vector<rct::key> amount_keys;
  tx.set_null();
  amount_keys.clear();
  if (context.d_msout) {
    context.d_msout->c.clear();
  }

  tx.version     = context.d_tx_version;
  tx.unlock_time = context.d_unlock_time;
  tx.extra       = context.d_extra;

  // if we have a stealth payment id, find it and encrypt it with the tx key now
  std::vector<tx_extra_field> tx_extra_fields;
  if (parse_tx_extra(tx.extra, tx_extra_fields))
  {
    tx_extra_nonce extra_nonce;
    if (find_tx_extra_field_by_type(tx_extra_fields, extra_nonce))
    {
      crypto::hash8 payment_id = null_hash8;
      if (get_encrypted_payment_id_from_tx_extra_nonce(extra_nonce.nonce, payment_id))
      {
        LOG_PRINT_L2("Encrypting payment id " << payment_id);
        crypto::public_key view_key_pub = get_destination_view_key_pub(context.d_destinations, context.d_change_addr);
        if (view_key_pub == crypto::NullKey::p())
        {
          LOG_ERROR("Destinations have to have exactly one output to support encrypted payment ids");
          return false;
        }

        if (!hwdev.encrypt_payment_id(payment_id, view_key_pub, context.d_tx_key))
        {
          LOG_ERROR("Failed to encrypt payment id");
          return false;
        }

        std::string extra_nonce;
        set_encrypted_payment_id_to_tx_extra_nonce(extra_nonce, payment_id);
        remove_field_from_tx_extra(tx.extra, typeid(tx_extra_nonce));
        if (!add_extra_nonce_to_tx_extra(tx.extra, extra_nonce))
        {
          LOG_ERROR("Failed to add encrypted payment id to tx extra");
          return false;
        }
        LOG_PRINT_L1("Encrypted payment ID: " << payment_id);
      }
    }
  }
  else
  {
    LOG_ERROR("Failed to parse tx extra");
    return false;
  }

  struct input_generation_context_data
  {
    keypair in_ephemeral;
  };
  std::vector<input_generation_context_data> in_contexts;

  uint64_t summary_inputs_money = 0;
  //fill inputs
  int idx = -1;
  for(const tx_source_entry &src_entr: context.d_sources)
  {
    ++idx;
    if(src_entr.real_output >= src_entr.outputs.size())
    {
      LOG_ERROR("real_output index (" << src_entr.real_output
                << ")bigger than output_keys.size()=" << src_entr.outputs.size());
      return false;
    }
    summary_inputs_money += src_entr.amount;

    //key_derivation recv_derivation;
    in_contexts.emplace_back();
    keypair& in_ephemeral = in_contexts.back().in_ephemeral;
    crypto::key_image img;
    const auto& out_key = reinterpret_cast<const crypto::public_key&>(src_entr.outputs[src_entr.real_output].second.dest);

    if (context.d_tgtx && src_entr.token_id != CUTCOIN_ID) {
      if (!generate_tgtx_key_image_helper(context.d_sender_account_keys,
                                          src_entr.real_out_tx_key,
                                          src_entr.real_output_in_tx_index,
                                          in_ephemeral,
                                          img,
                                          hwdev)) {
        LOG_ERROR("Key image generation failed!");
        return false;
      }
    } else {
      if (!generate_key_image_helper(context.d_sender_account_keys,
                                     context.d_subaddresses,
                                     out_key,
                                     src_entr.real_out_tx_key,
                                     src_entr.real_out_additional_tx_keys,
                                     src_entr.real_output_in_tx_index,
                                     in_ephemeral,
                                     img,
                                     hwdev)) {
        LOG_ERROR("Key image generation failed!");
        return false;
      }

      //check that derivated key is equal with real output key (if non multisig)
      if(!context.d_msout && !(in_ephemeral.pub == src_entr.outputs[src_entr.real_output].second.dest)) {
        LOG_ERROR("derived public key mismatch with output public key at index " << idx << ", real out " << src_entr.real_output << "! "<< ENDL << "derived_key:"
                                                                                 << string_tools::pod_to_hex(in_ephemeral.pub) << ENDL << "real output_public_key:"
                                                                                 << string_tools::pod_to_hex(src_entr.outputs[src_entr.real_output].second.dest) );
        LOG_ERROR("amount " << src_entr.amount << ", rct " << src_entr.rct);
        LOG_ERROR("tx pubkey " << src_entr.real_out_tx_key << ", real_output_in_tx_index " << src_entr.real_output_in_tx_index);
        return false;
      }
    }

    //put key image into tx input
    txin_to_key input_to_key;
    input_to_key.token_id = src_entr.token_id;
    input_to_key.amount = src_entr.amount;
    input_to_key.k_image = context.d_msout ? rct::rct2ki(src_entr.multisig_kLRki.ki) : img;

    //fill outputs array and use relative offsets
    for(const tx_source_entry::output_entry& out_entry: src_entr.outputs)
      input_to_key.key_offsets.push_back(out_entry.first);

    input_to_key.key_offsets = absolute_output_offsets_to_relative(input_to_key.key_offsets);
    tx.vin.emplace_back(input_to_key);
  }

//    if (shuffle_outs)
//    {
//      std::shuffle(destinations.begin(), destinations.end(), std::default_random_engine(crypto::rand<unsigned int>()));
//    }

  // sort ins by their key image
  std::vector<size_t> ins_order(context.d_sources.size());
  for (size_t n = 0; n < context.d_sources.size(); ++n)
    ins_order[n] = n;
  std::sort(ins_order.begin(), ins_order.end(), [&](const size_t i0, const size_t i1) {
    const txin_to_key &tk0 = boost::get<txin_to_key>(tx.vin[i0]);
    const txin_to_key &tk1 = boost::get<txin_to_key>(tx.vin[i1]);
    return (tk0.token_id == tk1.token_id)
           ? memcmp(&tk0.k_image, &tk1.k_image, sizeof(tk0.k_image)) > 0
           : tk0.token_id < tk1.token_id;
  });
  tools::apply_permutation(ins_order, [&] (size_t i0, size_t i1) {
    std::swap(tx.vin[i0], tx.vin[i1]);
    std::swap(in_contexts[i0], in_contexts[i1]);
    std::swap(const_cast<TxConstructionContext&>(context).d_sources[i0],
              const_cast<TxConstructionContext&>(context).d_sources[i1]);
  });

  // figure out if we need to make additional tx pubkeys
  size_t num_stdaddresses = 0;
  size_t num_subaddresses = 0;
  account_public_address single_dest_subaddress;
  classify_addresses(context.d_destinations,
                     context.d_change_addr,
                     num_stdaddresses,
                     num_subaddresses,
                     single_dest_subaddress);

  // if this is a single-destination transfer to a subaddress, we set the tx pubkey to R=s*D
  crypto::public_key txkey_pub;
  if (num_stdaddresses == 0 && num_subaddresses == 1)
  {
    txkey_pub = rct::rct2pk(hwdev.scalarmultKey(rct::pk2rct(single_dest_subaddress.m_spend_public_key),
                                                rct::sk2rct(context.d_tx_key)));
  }
  else
  {
    txkey_pub = rct::rct2pk(hwdev.scalarmultBase(rct::sk2rct(context.d_tx_key)));
  }
  remove_field_from_tx_extra(tx.extra, typeid(tx_extra_pub_key));
  add_tx_pub_key_to_extra(tx, txkey_pub);

  std::vector<crypto::public_key> additional_tx_public_keys;

  // we don't need to include additional tx keys if:
  //   - all the destinations are standard addresses
  //   - there's only one destination which is a subaddress
  bool need_additional_txkeys = num_subaddresses > 0 && (num_stdaddresses > 0 || num_subaddresses > 1);
  if (need_additional_txkeys)
    CHECK_AND_ASSERT_MES(context.d_destinations.size() == context.d_additional_tx_keys.size(),
                         false,
                         "Wrong amount of additional tx keys");

  uint64_t summary_outs_money = 0;
  //fill outputs
  size_t output_index = 0;
  for(const tx_destination_entry& dst_entr: context.d_destinations) {
    CHECK_AND_ASSERT_MES(dst_entr.amount > 0 || tx.version > TxVersion::plain,
                         false,
                         "Destination with wrong amount: " << dst_entr.amount);
    crypto::key_derivation derivation;
    crypto::public_key out_eph_public_key;

    // make additional tx pubkey if necessary
    keypair additional_txkey;
    if (need_additional_txkeys) {
      additional_txkey.sec = context.d_additional_tx_keys[output_index];
      if (dst_entr.is_subaddress)
        additional_txkey.pub = rct::rct2pk(hwdev.scalarmultKey(rct::pk2rct(dst_entr.addr.m_spend_public_key),
                                                               rct::sk2rct(additional_txkey.sec)));
      else
        additional_txkey.pub = rct::rct2pk(hwdev.scalarmultBase(rct::sk2rct(additional_txkey.sec)));
    }

    bool r;
    if (context.d_change_addr && dst_entr.addr == *context.d_change_addr) {
      // sending change to yourself; derivation = a*R
      r = hwdev.generate_key_derivation(txkey_pub, context.d_sender_account_keys.m_view_secret_key, derivation);
      CHECK_AND_ASSERT_MES(r, false, "at creation outs: failed to generate_key_derivation(" << txkey_pub
                                     << ", " << context.d_sender_account_keys.m_view_secret_key << ")");
    }
    else {
      // sending to the recipient; derivation = r*A (or s*C in the subaddress scheme)
      r = hwdev.generate_key_derivation(
          dst_entr.addr.m_view_public_key,
        dst_entr.is_subaddress && need_additional_txkeys ? additional_txkey.sec : context.d_tx_key, derivation);
        CHECK_AND_ASSERT_MES(r, false, "at creation outs: failed to generate_key_derivation("
                                       << dst_entr.addr.m_view_public_key << ", "
                                       << (dst_entr.is_subaddress && need_additional_txkeys ? additional_txkey.sec : context.d_tx_key) << ")");
    }

    if (need_additional_txkeys) {
      additional_tx_public_keys.push_back(additional_txkey.pub);
    }

    if (tx.version > TxVersion::plain) {
      crypto::secret_key scalar1;
      hwdev.derivation_to_scalar(derivation, output_index, scalar1);
      amount_keys.push_back(rct::sk2rct(scalar1));
    }
    r = hwdev.derive_public_key(derivation, output_index, dst_entr.addr.m_spend_public_key, out_eph_public_key);
    CHECK_AND_ASSERT_MES(r, false, "at creation outs: failed to derive_public_key(" << derivation
                                   << ", " << output_index << ", "<< dst_entr.addr.m_spend_public_key << ")");

    hwdev.add_output_key_mapping(dst_entr.addr.m_view_public_key,
                                 dst_entr.addr.m_spend_public_key,
                                 dst_entr.is_subaddress,
                                 output_index,
                                 amount_keys.back(),
                                 out_eph_public_key);

    tx_out out;
    out.token_id = dst_entr.token_id;
    out.amount = dst_entr.amount;
    txout_to_key tk;
    tk.key = out_eph_public_key;
    out.target = tk;
    tx.vout.push_back(out);
    output_index++;
    summary_outs_money += dst_entr.amount;
  }
  CHECK_AND_ASSERT_MES(additional_tx_public_keys.size() == context.d_additional_tx_keys.size(),
                       false,
                       "Internal error creating additional public keys");

  remove_field_from_tx_extra(tx.extra, typeid(tx_extra_additional_pub_keys));

  LOG_PRINT_L2("tx pubkey: " << txkey_pub);
  if (need_additional_txkeys) {
    LOG_PRINT_L2("additional tx pubkeys: ");
    for (size_t i = 0; i < additional_tx_public_keys.size(); ++i)
      LOG_PRINT_L2(additional_tx_public_keys[i]);
    add_additional_tx_pub_keys_to_extra(tx.extra, additional_tx_public_keys);
  }

  //check money
  if(summary_outs_money > summary_inputs_money) {
    LOG_ERROR("Transaction inputs money ("<< summary_inputs_money
              << ") less than outputs money (" << summary_outs_money << ")");
    return false;
  }

  // check for watch only wallet
  bool zero_secret_key = true;
  for (size_t i = 0; i < sizeof(context.d_sender_account_keys.m_spend_secret_key); ++i)
    zero_secret_key &= (context.d_sender_account_keys.m_spend_secret_key.data[i] == 0);
  if (zero_secret_key) {
    MDEBUG("Null secret key, skipping signatures");
  }

  if (tx.version == TxVersion::plain) {
    //generate ring signatures
    crypto::hash tx_prefix_hash;
    get_transaction_prefix_hash(tx, tx_prefix_hash);

    std::stringstream ss_ring_s;
    size_t i = 0;
    for(const tx_source_entry& src_entr: context.d_sources) {
      ss_ring_s << "pub_keys:" << ENDL;
      std::vector<const crypto::public_key*> keys_ptrs;
      std::vector<crypto::public_key> keys(src_entr.outputs.size());
      size_t ii = 0;
      for(const tx_source_entry::output_entry& o: src_entr.outputs)
      {
        keys[ii] = rct2pk(o.second.dest);
        keys_ptrs.push_back(&keys[ii]);
        ss_ring_s << o.second.dest << ENDL;
        ++ii;
      }

      tx.signatures.push_back(std::vector<crypto::signature>());
      std::vector<crypto::signature>& sigs = tx.signatures.back();
      sigs.resize(src_entr.outputs.size());
      if (!zero_secret_key)
        crypto::generate_ring_signature(tx_prefix_hash,
                                        boost::get<txin_to_key>(tx.vin[i]).k_image,
                                        keys_ptrs,
                                        in_contexts[i].in_ephemeral.sec,
                                        src_entr.real_output,
                                        sigs.data());
      ss_ring_s << "signatures:" << ENDL;
      std::for_each(sigs.begin(), sigs.end(), [&](const crypto::signature& s){ss_ring_s << s << ENDL;});
      ss_ring_s << "prefix_hash:" << tx_prefix_hash << ENDL
                << "in_ephemeral_key: " << in_contexts[i].in_ephemeral.sec << ENDL
                << "real_output: " << src_entr.real_output << ENDL;
      i++;
    }

    MCINFO("construct_tx", "transaction_created: " << get_transaction_hash(tx) << ENDL << obj_to_json_str(tx) << ENDL << ss_ring_s.str());
  }
  else {
    size_t n_total_outs = context.d_sources[0].outputs.size(); // only for non-simple rct

    // the non-simple version is slightly smaller, but assumes all real inputs
    // are on the same index, so can only be used if there just one ring.
    bool use_simple_rct = context.d_sources.size() > 1 || context.d_range_proof_type != rct::RangeProofBorromean;

    if (!use_simple_rct) {
      // non simple ringct requires all real inputs to be at the same index for all inputs
      for(const tx_source_entry& src_entr: context.d_sources) {
        if(src_entr.real_output != context.d_sources.begin()->real_output) {
          LOG_ERROR("All inputs must have the same index for non-simple ringct");
          return false;
        }
      }

      // enforce same mixin for all outputs
      for (size_t i = 1; i < context.d_sources.size(); ++i) {
        if (n_total_outs != context.d_sources[i].outputs.size()) {
          LOG_ERROR("Non-simple ringct transaction has varying ring size");
          return false;
        }
      }
    }

    std::map<TokenId, uint64_t> amount_in;
    std::map<TokenId, uint64_t> amount_out;
    rct::ctkeyV inSk;
    inSk.reserve(context.d_sources.size());
    // mixRing indexing is done the other way round for simple
    rct::ctkeyM mixRing(use_simple_rct ? context.d_sources.size() : n_total_outs);
    rct::keyV destinations;
    rct::ctamountV inamounts;
    std::map<TokenId, std::vector<uint64_t>> outamounts;
    std::vector<unsigned int> index;
    std::vector<rct::multisig_kLRki> kLRki;
    for (size_t i = 0; i < context.d_sources.size(); ++i) {
      const auto &token_id = context.d_sources[i].token_id;
      const auto &amount = context.d_sources[i].amount;
      amount_in[token_id] += amount;
      inamounts.push_back({amount, rct::tokenIdToPoint(token_id)});
      index.push_back(context.d_sources[i].real_output);

      // inSk: (secret key, mask)
      rct::ctkey ctkey;
      ctkey.dest = rct::sk2rct(in_contexts[i].in_ephemeral.sec);
      ctkey.mask = context.d_sources[i].mask;
      inSk.push_back(ctkey);
      memwipe(&ctkey, sizeof(rct::ctkey));
      // inPk: (public key, commitment)
      // will be done when filling in mixRing
      if (context.d_msout) {
        kLRki.push_back(context.d_sources[i].multisig_kLRki);
      }
    }

    for (size_t i = 0; i < tx.vout.size(); ++i) {
      destinations.push_back(rct::pk2rct(boost::get<txout_to_key>(tx.vout[i].target).key));
      const auto &token_id = tx.vout[i].token_id;
      const auto &amount = tx.vout[i].amount;
      outamounts[token_id].push_back(amount);
      amount_out[token_id] += amount;
    }

    if (use_simple_rct) {
      // mixRing indexing is done the other way round for simple
      for (size_t i = 0; i < context.d_sources.size(); ++i) {
        mixRing[i].resize(context.d_sources[i].outputs.size());
        for (size_t n = 0; n < context.d_sources[i].outputs.size(); ++n) {
          mixRing[i][n] = context.d_sources[i].outputs[n].second;
        }
      }
    }
    else {
      for (size_t i = 0; i < n_total_outs; ++i) {// same index assumption
        mixRing[i].resize(context.d_sources.size());
        for (size_t n = 0; n < context.d_sources.size(); ++n) {
          mixRing[i][n] = context.d_sources[n].outputs[i].second;
        }
      }
    }

    // fee
    if (!use_simple_rct && amount_in[CUTCOIN_ID] > amount_out[CUTCOIN_ID])
      outamounts[CUTCOIN_ID].push_back(amount_in[CUTCOIN_ID] - amount_out[CUTCOIN_ID]);

    // zero out all amounts to mask rct outputs, real amounts are now encrypted
    for (size_t i = 0; i < tx.vin.size(); ++i) {
      if (context.d_sources[i].rct)
        boost::get<txin_to_key>(tx.vin[i]).amount = 0;
    }

    for (auto& o : tx.vout)
      o.amount = 0;

    rct::ctkeyV outSk;
    crypto::hash tx_prefix_hash;

    auto calc_message = [&](const rct::key aG)-> rct::key {
      tx_extra_pos_stamp pos_stamp{};
      if (find_tx_extra_field_by_type(tx_extra_fields, pos_stamp)) {
        remove_field_from_tx_extra(tx.extra, typeid(tx_extra_pos_stamp));

        pos_stamp.key = aG;

        if (!add_pos_stamp_to_tx_extra(tx.extra, pos_stamp)) {
          LOG_ERROR("Failed to add POS stamp to tx extra");
        } else {
          LOG_PRINT_L1("Constructed stake transaction");
        }
      }

      get_transaction_prefix_hash(tx, tx_prefix_hash);
      return rct::hash2rct(tx_prefix_hash);
    };

    if (use_simple_rct) {
      if (tx.version != TxVersion::tokens) {
        tx.rct_signatures = rct::genRctSimple(calc_message,
                                              inSk,
                                              destinations,
                                              inamounts,
                                              outamounts[CUTCOIN_ID],
                                              amount_in[CUTCOIN_ID] - amount_out[CUTCOIN_ID],
                                              mixRing,
                                              amount_keys,
                                              context.d_msout ? &kLRki: nullptr,
                                              context.d_msout,
                                              index,
                                              outSk,
                                              context.d_range_proof_type,
                                              hwdev);
      } else {
        tx.rct_signatures = rct::genRctSimpleBig(calc_message,
                                                 inSk,
                                                 destinations,
                                                 inamounts,
                                                 outamounts,
                                                 amount_in[CUTCOIN_ID] - amount_out[CUTCOIN_ID],
                                                 mixRing,
                                                 amount_keys,
                                                 context.d_msout ? &kLRki: nullptr,
                                                 context.d_msout,
                                                 index,
                                                 outSk,
                                                 hwdev);
      }
    } else {
      get_transaction_prefix_hash(tx, tx_prefix_hash);
      tx.rct_signatures = rct::genRct(rct::hash2rct(tx_prefix_hash),
                                      inSk,
                                      destinations,
                                      outamounts[CUTCOIN_ID],
                                      mixRing,
                                      amount_keys,
                                      context.d_msout ? &kLRki[0]: nullptr,
                                      context.d_msout,
                                      context.d_sources[0].real_output,
                                      outSk,
                                      hwdev); // same index assumption
    }
    memwipe(inSk.data(), inSk.size() * sizeof(rct::ctkey));

    CHECK_AND_ASSERT_MES(tx.vout.size() == outSk.size(), false, "outSk size does not match vout");

    MCINFO("construct_tx", "transaction_created: " << get_transaction_hash(tx) << ENDL << obj_to_json_str(tx) << ENDL);
  }

  tx.invalidate_hashes();

  return true;
}

bool construct_token_tx_with_tx_key(TxConstructionContext &context, transaction &tx)
{
  hw::device &hwdev = context.d_sender_account_keys.get_device();

  if (context.d_sources.empty()) {
    LOG_ERROR("Empty sources");
    return false;
  }

  std::vector<rct::key> amount_keys;
  tx.set_null();
  amount_keys.clear();
  tx.version = context.d_tx_version;
  tx.set_token_genesis(true);
  tx.unlock_time = context.d_unlock_time;

  tx.extra = context.d_extra;

  struct input_generation_context_data
  {
    keypair in_ephemeral;
  };
  std::vector<input_generation_context_data> in_contexts;

  uint64_t summary_inputs_money = 0;
  //fill inputs
  int idx = -1;
  for(const tx_source_entry &src_entr: context.d_sources) {
    ++idx;
    if(src_entr.real_output >= src_entr.outputs.size()) {
      LOG_ERROR("real_output index (" << src_entr.real_output << ")bigger than output_keys.size()=" << src_entr.outputs.size());
      return false;
    }
    summary_inputs_money += src_entr.amount;

    //key_derivation recv_derivation;
    in_contexts.push_back(input_generation_context_data());
    keypair& in_ephemeral = in_contexts.back().in_ephemeral;
    crypto::key_image img;
    const auto &out_key = reinterpret_cast<const crypto::public_key&>(src_entr.outputs[src_entr.real_output].second.dest);

    if(src_entr.token_id != CUTCOIN_ID) {
      generate_tgtx_key_image_helper(context.d_sender_account_keys,
                                     src_entr.real_out_tx_key,
                                     src_entr.real_output_in_tx_index,
                                     in_ephemeral,
                                     img,
                                     hwdev);
      crypto::key_derivation recv_derivation = AUTO_VAL_INIT(recv_derivation);
      bool r = hwdev.generate_key_derivation(src_entr.real_out_tx_key,
                                             context.d_sender_account_keys.m_view_secret_key,
                                             recv_derivation);
      if (!r) {
        MWARNING("key image helper: failed to generate_key_derivation(" << src_entr.real_out_tx_key << ", " << context.d_sender_account_keys.m_view_secret_key << ")");
        memcpy(&recv_derivation, rct::identity().bytes, sizeof(recv_derivation));
      }

      r = hwdev.derive_secret_key(recv_derivation, src_entr.real_output_in_tx_index, context.d_sender_account_keys.m_spend_secret_key, in_ephemeral.sec);
      r = hwdev.secret_key_to_public_key(in_ephemeral.sec, in_ephemeral.pub);

      crypto::generate_key_image(in_ephemeral.pub, in_ephemeral.sec, img);
    } else {
      if (!generate_key_image_helper(context.d_sender_account_keys,
                                     context.d_subaddresses,
                                     out_key,
                                     src_entr.real_out_tx_key,
                                     src_entr.real_out_additional_tx_keys,
                                     src_entr.real_output_in_tx_index,
                                     in_ephemeral, img, hwdev)) {
        LOG_ERROR("Key image generation failed!");
        return false;
      }

      //check that derived key is equal to real output key (if non multisig)
      if(!(in_ephemeral.pub == src_entr.outputs[src_entr.real_output].second.dest) )
      {
        LOG_ERROR("derived public key mismatch with output public key at index " << idx
                  << ", real out " << src_entr.real_output << "! "<< ENDL << "derived_key:"
                  << string_tools::pod_to_hex(in_ephemeral.pub) << ENDL << "real output_public_key:"
                  << string_tools::pod_to_hex(src_entr.outputs[src_entr.real_output].second.dest) );
        LOG_ERROR("amount " << src_entr.amount << ", rct " << src_entr.rct);
        LOG_ERROR("tx pubkey " << src_entr.real_out_tx_key
                  << ", real_output_in_tx_index " << src_entr.real_output_in_tx_index);
        return false;
      }
    }

    //put key image into tx input
    txin_to_key input_to_key;
    input_to_key.token_id = src_entr.token_id;
    input_to_key.amount = src_entr.amount;
    input_to_key.k_image = img;

    //fill outputs array and use relative offsets
    for(const tx_source_entry::output_entry& out_entry: src_entr.outputs)
      input_to_key.key_offsets.push_back(out_entry.first);

    input_to_key.key_offsets = absolute_output_offsets_to_relative(input_to_key.key_offsets);
    tx.vin.emplace_back(input_to_key);
  }

  // Sort ins by their key image and token id.
  std::vector<size_t> ins_order(context.d_sources.size());
  for (size_t n = 0; n < context.d_sources.size(); ++n)
    ins_order[n] = n;
  std::sort(ins_order.begin(), ins_order.end(), [&](const size_t i0, const size_t i1) {
    const txin_to_key &tk0 = boost::get<txin_to_key>(tx.vin[i0]);
    const txin_to_key &tk1 = boost::get<txin_to_key>(tx.vin[i1]);
    return (tk0.token_id == tk1.token_id)
        ? memcmp(&tk0.k_image, &tk1.k_image, sizeof(tk0.k_image)) > 0
        : tk0.token_id < tk1.token_id;
  });
  tools::apply_permutation(ins_order, [&] (size_t i0, size_t i1) {
    std::swap(tx.vin[i0], tx.vin[i1]);
    std::swap(in_contexts[i0], in_contexts[i1]);
    std::swap(context.d_sources[i0], context.d_sources[i1]);
  });

  // figure out if we need to make additional tx pubkeys
  size_t num_stdaddresses = 0;
  size_t num_subaddresses = 0;
  account_public_address single_dest_subaddress{};
  classify_addresses(context.d_destinations,
                     context.d_change_addr,
                     num_stdaddresses,
                     num_subaddresses,
                     single_dest_subaddress);

  crypto::public_key txkey_pub;

  // if this is a single-destination transfer to a subaddress, we set the tx pubkey to R=s*D
  if (num_stdaddresses == 0 && num_subaddresses == 1){
    txkey_pub = rct::rct2pk(hwdev.scalarmultKey(rct::pk2rct(single_dest_subaddress.m_spend_public_key),
                                                rct::sk2rct(context.d_tx_key)));
  }
  else {
    txkey_pub = rct::rct2pk(hwdev.scalarmultBase(rct::sk2rct(context.d_tx_key)));
  }
  remove_field_from_tx_extra(tx.extra, typeid(tx_extra_pub_key));
  add_tx_pub_key_to_extra(tx, txkey_pub);

  std::vector<crypto::public_key> additional_tx_public_keys;

  // we don't need to include additional tx keys if:
  //   - all the destinations are standard addresses
  //   - there's only one destination which is a subaddress
  bool need_additional_txkeys = num_subaddresses > 0 && (num_stdaddresses > 0 || num_subaddresses > 1);
  if (need_additional_txkeys)
    CHECK_AND_ASSERT_MES(context.d_destinations.size() == context.d_additional_tx_keys.size(),
                         false,
                         "Wrong amount of additional tx keys");

  //fill outputs
  size_t output_index = 0;
  for (const tx_destination_entry &dst: context.d_destinations) {
    if (dst.token_id == CUTCOIN_ID) {
      CHECK_AND_ASSERT_MES(dst.amount > 0 || tx.version > TxVersion::plain,
                           false,
                           "Destination with wrong amount: " << dst.amount);
      crypto::key_derivation derivation;
      crypto::public_key out_eph_public_key;

      // make additional tx pubkey if necessary
      keypair additional_txkey;

      bool r;
      if (context.d_change_addr && dst.addr == *(context.d_change_addr)) {
        // sending change to yourself; derivation = a*R
        r = hwdev.generate_key_derivation(txkey_pub, context.d_sender_account_keys.m_view_secret_key, derivation);
        CHECK_AND_ASSERT_MES(r, false, "at creation outs: failed to generate_key_derivation(" << txkey_pub
                                       << ", " << context.d_sender_account_keys.m_view_secret_key << ")");
      }
      else {
        // sending to the recipient; derivation = r*A (or s*C in the subaddress scheme)
        r = hwdev.generate_key_derivation(dst.addr.m_view_public_key,
                                          dst.is_subaddress && need_additional_txkeys ? additional_txkey.sec : context.d_tx_key, derivation);
        CHECK_AND_ASSERT_MES(r, false, "at creation outs: failed to generate_key_derivation("
                                       << dst.addr.m_view_public_key <<
                                       ", " << (dst.is_subaddress && need_additional_txkeys ? additional_txkey.sec : context.d_tx_key) << ")");
      }

      crypto::secret_key scalar1;
      hwdev.derivation_to_scalar(derivation, output_index, scalar1);
      amount_keys.push_back(rct::sk2rct(scalar1));

      r = hwdev.derive_public_key(derivation, output_index, dst.addr.m_spend_public_key, out_eph_public_key);
      CHECK_AND_ASSERT_MES(r, false, "at creation outs: failed to derive_public_key(" << derivation
                                     << ", " << output_index << ", "<< dst.addr.m_spend_public_key << ")");

      hwdev.add_output_key_mapping(dst.addr.m_view_public_key,
                                   dst.addr.m_spend_public_key,
                                   dst.is_subaddress,
                                   output_index,
                                   amount_keys.back(),
                                   out_eph_public_key);

      tx_out out;
      out.token_id = CUTCOIN_ID;
      out.amount = dst.amount;
      txout_to_key tk;
      tk.key = out_eph_public_key;
      out.target = tk;
      tx.vout.push_back(out);
      output_index++;
    } else {
      crypto::key_derivation derivation;
      crypto::public_key out_eph_public_key;

      std::vector<TokenUnit> out_amounts;
      decompose_token_supply(dst.amount, out_amounts);

      for (const TokenUnit out_amount: out_amounts) {
        bool r = hwdev.generate_key_derivation(txkey_pub, context.d_sender_account_keys.m_view_secret_key, derivation);
        CHECK_AND_ASSERT_MES(r, false, "at creation outs: failed to generate_key_derivation(" << txkey_pub
                                       << ", " << context.d_sender_account_keys.m_view_secret_key << ")");

        crypto::secret_key scalar1;
        hwdev.derivation_to_scalar(derivation, output_index, scalar1);
        amount_keys.push_back(rct::sk2rct(scalar1));

        r = hwdev.derive_public_key(derivation, output_index, dst.addr.m_spend_public_key, out_eph_public_key);
        CHECK_AND_ASSERT_MES(r, false, "at creation outs: failed to derive_public_key(" << derivation
                                       << ", " << output_index << ", "<< dst.addr.m_spend_public_key << ")");

        hwdev.add_output_key_mapping(dst.addr.m_view_public_key,
                                     dst.addr.m_spend_public_key,
                                     dst.is_subaddress,
                                     output_index,
                                     amount_keys.back(),
                                     out_eph_public_key);

        tx_out out;
        out.token_id = dst.token_id;
        out.amount = out_amount;
        txout_to_key tk;
        tk.key = out_eph_public_key;
        out.target = tk;
        tx.vout.push_back(out);
        output_index++;
      }
    }
  }
  CHECK_AND_ASSERT_MES(additional_tx_public_keys.size() == context.d_additional_tx_keys.size(),
                       false,
                       "Internal error creating additional public keys");

  {
    rct::ctkeyV inSk;
    inSk.reserve(context.d_sources.size());
    // mixRing indexing is done the other way round for simple
    rct::ctkeyM mixRing(context.d_sources.size());
    rct::keyV destinations;
    rct::ctamountV inamounts;
    std::map<TokenId, std::vector<uint64_t>> outamounts;
    std::vector<unsigned int> index;
    std::vector<rct::multisig_kLRki> kLRki;
    uint64_t c_amount_in = 0, c_amount_out = 0, c_amount_fee = 0;
    uint64_t t_amount_in = 0, t_amount_out = 0;

    for (size_t i = 0; i < context.d_sources.size(); ++i) {
      rct::ctkey ctkey{};
      if (context.d_sources[i].token_id == CUTCOIN_ID) {
        c_amount_in += context.d_sources[i].amount;
      } else {
        t_amount_in += context.d_sources[i].amount;
      }
      inamounts.push_back({context.d_sources[i].amount, rct::tokenIdToPoint(context.d_sources[i].token_id)});
      index.push_back(context.d_sources[i].real_output);
      // inSk: (secret key, mask)
      ctkey.dest = rct::sk2rct(in_contexts[i].in_ephemeral.sec);
      ctkey.mask = context.d_sources[i].mask;
      inSk.push_back(ctkey);
      memwipe(&ctkey, sizeof(rct::ctkey));
    }

    for (size_t i = 0; i < tx.vout.size(); ++i) {
      destinations.push_back(rct::pk2rct(boost::get<txout_to_key>(tx.vout[i].target).key));
      outamounts[tx.vout[i].token_id].push_back(tx.vout[i].amount);
      if (tx.vout[i].token_id == CUTCOIN_ID) {
        c_amount_out += tx.vout[i].amount;
      } else {
        t_amount_out += tx.vout[i].amount;
      }
    }

    MCINFO("construct_token_tx_with_tx_key", "Summary token supply: " << t_amount_out << std::endl);

    c_amount_fee = c_amount_in - c_amount_out;

    // mixRing indexing is done the other way round for simple
    for (size_t i = 0; i < context.d_sources.size(); ++i) {
      mixRing[i].resize(context.d_sources[i].outputs.size());
      for (size_t n = 0; n < context.d_sources[i].outputs.size(); ++n) {
        mixRing[i][n] = context.d_sources[i].outputs[n].second;
      }
    }

    // zero out all amounts to mask rct outputs, real amounts are now encrypted
    for (size_t i = 0; i < tx.vin.size(); ++i) {
      if (context.d_sources[i].rct)
        boost::get<txin_to_key>(tx.vin[i]).amount = 0;
    }
    for (auto & i : tx.vout)
      i.amount = 0;

    rct::ctkeyV outSk;
    crypto::hash tx_prefix_hash{};

    auto calc_message = [&](const rct::key aG) -> rct::key {
      get_transaction_prefix_hash(tx, tx_prefix_hash);
      return rct::hash2rct(tx_prefix_hash);
    };

    CHECK_AND_ASSERT_MES(outamounts.size() == 2, false, "outamounts size does not equal 2");

    tx.rct_signatures = rct::genRctTgtx(calc_message,
                                        context.d_hidden_supply,
                                        inSk,
                                        destinations,
                                        inamounts,
                                        outamounts,
                                        c_amount_fee,
                                        mixRing,
                                        amount_keys,
                                        nullptr,
                                        nullptr,
                                        index,
                                        outSk,
                                        hwdev);

    memwipe(inSk.data(), inSk.size() * sizeof(rct::ctkey));

    CHECK_AND_ASSERT_MES(tx.vout.size() == outSk.size(), false, "outSk size does not match vout");

    MCINFO("construct_tx", "transaction_created: " << get_transaction_hash(tx) << ENDL << obj_to_json_str(tx) << ENDL);
  }

  tx.invalidate_hashes();

  return true;
}

bool construct_tx_and_get_tx_key(TxConstructionContext &context, transaction &tx)
{
  hw::device &hwdev = context.d_sender_account_keys.get_device();
  hwdev.open_tx(context.d_tx_key);

  // figure out if we need to make additional tx pubkeys
  size_t num_stdaddresses = 0;
  size_t num_subaddresses = 0;
  account_public_address single_dest_subaddress;
  classify_addresses(context.d_destinations,
                     context.d_change_addr,
                     num_stdaddresses,
                     num_subaddresses,
                     single_dest_subaddress);
  bool need_additional_txkeys = num_subaddresses > 0 && (num_stdaddresses > 0 || num_subaddresses > 1);
  if (need_additional_txkeys) {
    context.d_additional_tx_keys.clear();
    for (const auto &d: context.d_destinations)
      context.d_additional_tx_keys.push_back(keypair::generate(context.d_sender_account_keys.get_device()).sec);
  }

  bool r = construct_tx_with_tx_key(context, tx);
  hwdev.close_tx();
  return r;
}

bool construct_token_tx_and_get_tx_key(TxConstructionContext &context, transaction &tx)
{
  hw::device &hwdev = context.d_sender_account_keys.get_device();
  hwdev.open_tx(context.d_tx_key);

  // figure out if we need to make additional tx pubkeys
  size_t num_stdaddresses = 0;
  size_t num_subaddresses = 0;
  account_public_address single_dest_subaddress;
  classify_addresses(context.d_destinations,
                     context.d_change_addr,
                     num_stdaddresses,
                     num_subaddresses,
                     single_dest_subaddress);
  bool need_additional_txkeys = num_subaddresses > 0 && (num_stdaddresses > 0 || num_subaddresses > 1);
  if (need_additional_txkeys) {
    context.d_additional_tx_keys.clear();
    for (const auto &d: context.d_destinations)
      context.d_additional_tx_keys.push_back(keypair::generate(context.d_sender_account_keys.get_device()).sec);
  }

  bool r = construct_token_tx_with_tx_key(context, tx);
  hwdev.close_tx();
  return r;
}

bool construct_tx(const account_keys                            &sender_account_keys,
                  TxSources                                     &sources,
                  const std::vector<tx_destination_entry>       &destinations,
                  const boost::optional<account_public_address> &change_addr,
                  std::vector<uint8_t>                           extra,
                  transaction                                   &tx,
                  uint64_t                                       unlock_time)
{
  std::unordered_map<crypto::public_key, subaddress_index> subaddresses;
  subaddresses[sender_account_keys.m_account_address.m_spend_public_key] = {0,0};
  std::vector<tx_destination_entry> destinations_copy = destinations;
  std::vector<crypto::secret_key> additional_tx_keys;

  TxConstructionContext context;
  context.d_sender_account_keys = sender_account_keys;
  context.d_subaddresses        = subaddresses;
  context.d_sources             = sources;
  context.d_destinations        = destinations;
  context.d_change_addr         = change_addr;
  context.d_extra               = std::move(extra);
  context.d_unlock_time         = unlock_time;
  context.d_additional_tx_keys  = additional_tx_keys;
  context.d_tx_version          = TxVersion::plain;
  context.d_range_proof_type    = rct::RangeProofBorromean;

  return construct_tx_and_get_tx_key(context, tx);
}

bool generate_genesis_block(block &bl, const std::string &genesis_tx , uint32_t nonce)
{
  //genesis block
  bl = boost::value_initialized<block>();

  blobdata tx_bl;
  bool r = string_tools::parse_hexstr_to_binbuff(genesis_tx, tx_bl);
  CHECK_AND_ASSERT_MES(r, false, "failed to parse coinbase tx from hard coded blob");
  r = parse_and_validate_tx_from_blob(tx_bl, bl.miner_tx);
  CHECK_AND_ASSERT_MES(r, false, "failed to parse coinbase tx from hard coded blob");
  bl.major_version = CURRENT_BLOCK_MAJOR_VERSION;
  bl.minor_version = CURRENT_BLOCK_MINOR_VERSION;
  bl.timestamp = 0;
  bl.nonce = nonce;
  miner::find_nonce_for_given_block(bl, 1, 0);
  bl.invalidate_hashes();
  return true;
}

void decompose_token_supply(TokenUnit token_supply, std::vector<TokenUnit> &out_amounts)
{
  static_assert(TOKEN_GENESIS_OUTPUTS != 0, "TOKEN_GENESIS_OUTPUTS cannot be equal to 0");
  TokenUnit out_amount = token_supply / TOKEN_GENESIS_OUTPUTS;
  out_amounts.clear();
  for (size_t i = 0; i < TOKEN_GENESIS_OUTPUTS - 1; ++i) {
    out_amounts.emplace_back(out_amount);
  }

  out_amount = token_supply - out_amount * (TOKEN_GENESIS_OUTPUTS - 1);
  out_amounts.emplace_back(out_amount);
}

} // namespace cryptonote
