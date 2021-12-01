// Copyright (c) 2021, CUT coin
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

#include "chaingen.h"
#include "cryptonote_basic/account.h"
#include "cryptonote_basic/cryptonote_basic.h"
#include "cryptonote_basic/cryptonote_format_utils.h"
#include <cryptonote_core/cryptonote_tx_utils.h>
#include "cryptonote_core/tx_construction_context.h"
#include "cryptonote_core/tx_destination_entry.h"
#include "cryptonote_core/tx_source_entry.h"
#include <ringct/rctSigs.h>

#include "gtest/gtest.h"

#include <chrono>
#include <vector>

using namespace cryptonote;

#define GENERATE_ACCOUNT(account)                     \
  account_base account;                               \
  account.generate();

#define MAKE_GENESIS_BLOCK(BLK_NAME, MINER_ACC, TS)   \
  test_generator generator{};                         \
  cryptonote::block BLK_NAME;                         \
  generator.construct_block(BLK_NAME, MINER_ACC, TS); \

void generate_dummy(transaction &tx, const int *out_idx, int mixin, uint64_t amount_paid)
{
  uint64_t amount             = 30000000000000;
  uint64_t fake_outputs_count = 10;
  uint64_t ts_start           = 1234567890;

  cryptonote::account_base miner_account;
  miner_account.generate();

  MAKE_GENESIS_BLOCK(blk_0, miner_account, ts_start);

  // create 4 miner accounts, and have them mine the next 4 blocks
  const cryptonote::block *prev_block = &blk_0;
  cryptonote::block blocks[fake_outputs_count];
  for (size_t n = 0; n < fake_outputs_count; ++n) {
    ASSERT_TRUE(generator.construct_block_manually(
      blocks[n], *prev_block, miner_account,
      test_generator::bf_major_ver | test_generator::bf_minor_ver | test_generator::bf_timestamp | test_generator::bf_hf_version,
      2, 2, prev_block->timestamp + DIFFICULTY_BLOCKS_ESTIMATE_TIMESPAN * 2, // v2 has blocks twice as long
      crypto::hash(), 0, transaction(), std::vector<crypto::hash>(), 0, 0, 2));
    prev_block = blocks + n;
    LOG_PRINT_L0("Initial miner tx " << n << ": " << obj_to_json_str(blocks[n].miner_tx));
  }

  //prepare token inputs
  {
    account_public_address address = miner_account.get_keys().m_account_address;
    keypair pseudo_tx_key = keypair::generate(miner_account.get_device());
    crypto::key_derivation derivation = AUTO_VAL_INIT(derivation);

    ASSERT_TRUE(crypto::generate_key_derivation(address.m_view_public_key, pseudo_tx_key.sec, derivation));

    const size_t output_index = 0;
    crypto::public_key pseudo_out_pub_key = AUTO_VAL_INIT(pseudo_out_pub_key);
    ASSERT_TRUE(crypto::derive_public_key(derivation, output_index, address.m_spend_public_key, pseudo_out_pub_key));

    tx_source_entry pseudo_source;
    pseudo_source.token_id = CUTCOIN_ID;
    pseudo_source.amount = amount;
    pseudo_source.real_out_tx_key = pseudo_tx_key.pub;
    pseudo_source.real_output = 0;
    pseudo_source.real_output_in_tx_index = 0;
    pseudo_source.mask = rct::identity();
    pseudo_source.rct = false;
    pseudo_source.push_output(0, pseudo_out_pub_key, pseudo_source.amount, pseudo_source.token_id);

    for (size_t m = 0; m < fake_outputs_count; ++m) {
      pseudo_source.push_output(m + 1, boost::get<txout_to_key>(blocks[m].miner_tx.vout[0].target).key, pseudo_source.amount);
    }

    std::vector<tx_source_entry> pseudo_sources{pseudo_source};

    //fill outputs entry
    tx_destination_entry td;
    td.token_id = CUTCOIN_ID;
    td.addr = address;
    td.amount = amount;
    std::vector<tx_destination_entry> pseudo_destinations{td};

    std::unordered_map<crypto::public_key, subaddress_index> subaddresses;
    subaddresses[address.m_spend_public_key] = {0, 0};
    transaction tmp_tx;

    {
      TxConstructionContext context;
      context.d_sender_account_keys = miner_account.get_keys();
      context.d_subaddresses = subaddresses;
      context.d_sources = pseudo_sources;
      context.d_destinations = pseudo_destinations;
//      context.d_change_addr = account_public_address{};
      context.d_tx_version = TxVersion::tokens;
      context.d_range_proof_type = rct::RangeProofType::RangeProofPaddedBulletproof;

      bool r = construct_tx_and_get_tx_key(context, tmp_tx);

      ASSERT_TRUE(r);
      ASSERT_EQ(tmp_tx.vout.size(), 1);

      ASSERT_TRUE(crypto::generate_key_derivation(address.m_view_public_key, context.d_tx_key, derivation));
    }

    crypto::secret_key amount_key;
    crypto::derivation_to_scalar(derivation, 0, amount_key);
    rct::key mask{};
    std::vector<rct::key> points{rct::tokenIdToPoint(CUTCOIN_ID)};
    rct::xmr_amount sent_amount = rct::decodeRctSimple(tmp_tx.rct_signatures,
                                                       rct::sk2rct(amount_key),
                                                       points,
                                                       0,
                                                       mask,
                                                       hw::get_device("default"));
    ASSERT_EQ(sent_amount, amount);

    // measurement
    {
      using namespace std::chrono;

      size_t num_repetitions = 1000;

      auto t1 = high_resolution_clock::now();
      for (size_t i = 0; i < num_repetitions; ++i) {
        rct::decodeRctSimple(tmp_tx.rct_signatures,
                             rct::sk2rct(amount_key),
                             points,
                             0,
                             mask,
                             miner_account.get_device());
      }
      auto t2 = high_resolution_clock::now();
      duration<double, std::milli> ms_double = t2 - t1;
      double single_call_duration = ms_double.count() / num_repetitions;
      std::cout << "Single decode call duration is " << single_call_duration << std::endl;
    }

  }
}

TEST(dex, special_address_creation_speed) {
  transaction dummy_tx;
  const int out_idx[] = {1, -1};
  const int mixin = 2;
  const uint64_t amount_paid = 10000;

  generate_dummy(dummy_tx, out_idx, mixin, amount_paid);
}