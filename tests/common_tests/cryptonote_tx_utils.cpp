// Copyright (c) 2020-2022, CUT coin
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

#include "cryptonote_basic/account.h"
#include "cryptonote_core/cryptonote_tx_utils.h"

#include "gtest/gtest.h"

TEST(cryptonote_tx_utils, basic)
{
//  using namespace cryptonote;
//  using namespace testing;
//
//
//  account_base miner_acc1;
//  miner_acc1.generate();
//  account_base miner_acc2;
//  miner_acc2.generate();
//  account_base miner_acc3;
//  miner_acc3.generate();
//  account_base miner_acc4;
//  miner_acc4.generate();
//  account_base miner_acc5;
//  miner_acc5.generate();
//  account_base miner_acc6;
//  miner_acc6.generate();
//
//  std::string add_str = miner_acc3.get_public_address_str(MAINNET);
//
//
//  account_base rv_acc;
////  rv_acc.generate();
//  account_public_address address;
//  crypto::secret_key     view_secret_key;
//  get_coin_burn_address(address, view_secret_key);
//  rv_acc.create_from_viewkey(address, view_secret_key);
//
//
//  account_base rv_acc2;
//  rv_acc2.generate();
//  transaction tx_mine_1;
//  construct_miner_tx(0, 0, 0, 10, 0, miner_acc1.get_keys().m_account_address, tx_mine_1);
//  transaction tx_mine_2;
//  construct_miner_tx(0, 0, 0, 0, 0, miner_acc2.get_keys().m_account_address, tx_mine_2);
//  transaction tx_mine_3;
//  construct_miner_tx(0, 0, 0, 0, 0, miner_acc3.get_keys().m_account_address, tx_mine_3);
//  transaction tx_mine_4;
//  construct_miner_tx(0, 0, 0, 0, 0, miner_acc4.get_keys().m_account_address, tx_mine_4);
//  transaction tx_mine_5;
//  construct_miner_tx(0, 0, 0, 0, 0, miner_acc5.get_keys().m_account_address, tx_mine_5);
//  transaction tx_mine_6;
//  construct_miner_tx(0, 0, 0, 0, 0, miner_acc6.get_keys().m_account_address, tx_mine_6);
//
//  //fill inputs entry
//  typedef tx_source_entry::output_entry tx_output_entry;
//  std::vector<tx_source_entry> sources;
//  sources.resize(sources.size()+1);
//  tx_source_entry& src = sources.back();
//  src.amount = 70368744177663;
//  {
//    tx_output_entry oe;
//
//    src.push_output(0, boost::get<txout_to_key>(tx_mine_1.vout[0].target).key, src.amount);
//
//    src.push_output(1, boost::get<txout_to_key>(tx_mine_2.vout[0].target).key, src.amount);
//
//    src.push_output(2, boost::get<txout_to_key>(tx_mine_3.vout[0].target).key, src.amount);
//
//    src.push_output(3, boost::get<txout_to_key>(tx_mine_4.vout[0].target).key, src.amount);
//
//    src.push_output(4, boost::get<txout_to_key>(tx_mine_5.vout[0].target).key, src.amount);
//
//    src.push_output(5, boost::get<txout_to_key>(tx_mine_6.vout[0].target).key, src.amount);
//
//    src.real_out_tx_key = cryptonote::get_tx_pub_key_from_extra(tx_mine_2);
//    src.real_output = 1;
//    src.rct = false;
//    src.real_output_in_tx_index = 0;
//  }
//  //fill outputs entry
//  tx_destination_entry td;
//  td.addr = rv_acc.get_keys().m_account_address;
//  td.amount = 69368744177663;
//  std::vector<tx_destination_entry> destinations;
//  destinations.push_back(td);
//
//  transaction tx_rc1;
//  bool r = construct_tx(miner_acc2.get_keys(), sources, destinations, boost::none, std::vector<uint8_t>(), tx_rc1, 0);
//  ASSERT_TRUE(r);
//
//  crypto::hash pref_hash = get_transaction_prefix_hash(tx_rc1);
//  std::vector<const crypto::public_key *> output_keys;
//  output_keys.push_back(&boost::get<txout_to_key>(tx_mine_1.vout[0].target).key);
//  output_keys.push_back(&boost::get<txout_to_key>(tx_mine_2.vout[0].target).key);
//  output_keys.push_back(&boost::get<txout_to_key>(tx_mine_3.vout[0].target).key);
//  output_keys.push_back(&boost::get<txout_to_key>(tx_mine_4.vout[0].target).key);
//  output_keys.push_back(&boost::get<txout_to_key>(tx_mine_5.vout[0].target).key);
//  output_keys.push_back(&boost::get<txout_to_key>(tx_mine_6.vout[0].target).key);
//  r = crypto::check_ring_signature(pref_hash, boost::get<txin_to_key>(tx_rc1.vin[0]).k_image, output_keys, &tx_rc1.signatures[0][0]);
//  ASSERT_TRUE(r);
//
//  std::vector<size_t> outs;
//  uint64_t money = 0;
//
//  r = lookup_acc_outs(rv_acc.get_keys(), tx_rc1, get_tx_pub_key_from_extra(tx_rc1), get_additional_tx_pub_keys_from_extra(tx_rc1), outs,  money);
//  ASSERT_TRUE(r);
//  ASSERT_EQ(td.amount, money);
//
//  money = 0;
//  r = lookup_acc_outs(rv_acc2.get_keys(), tx_rc1, get_tx_pub_key_from_extra(tx_rc1), get_additional_tx_pub_keys_from_extra(tx_rc1), outs,  money);
//  ASSERT_TRUE(r);
//  ASSERT_EQ(0, money);
}
