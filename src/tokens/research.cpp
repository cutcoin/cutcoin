// Copyright (c) 2019-2022, CUT coin
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

#include "research.h"

#include "blockchain_db/blockchain_db.h"
#include "blockchain_db/lmdb/db_lmdb.h"
#include "cryptonote_basic/blobdatatype.h"
#include "cryptonote_basic/cryptonote_basic.h"
#include "cryptonote_basic/cryptonote_format_utils.h"
#include "cryptonote_basic/difficulty.h"
#include "cryptonote_basic/verification_context.h"
#include "cryptonote_core/cryptonote_core.h"
#include "device/device.hpp"
#include "ringct/rctSigs.h"
#include "ringct/rctTypes.h"
#include "string_tools.h"
#include <wallet/wallet2.h>

#include <boost/filesystem.hpp>

#include <iostream>
#include <string>
#include <unistd.h>

// Test genRct -> verRct pair

void generate_verify()
{

#if 0
  rct::ctkeyV sc, pc;
  rct::ctkey sctmp, pctmp;
  std::vector<unsigned int> index;
  std::vector<uint64_t> inamounts, outamounts;

  std::tie(sctmp, pctmp) = rct::ctskpkGen(6000);
  sc.push_back(sctmp);
  pc.push_back(pctmp);
  inamounts.push_back(6000);
  index.push_back(1);

  std::tie(sctmp, pctmp) = rct::ctskpkGen(7000);
  sc.push_back(sctmp);
  pc.push_back(pctmp);
  inamounts.push_back(7000);
  index.push_back(1);

  const size_t max_outputs = 16;

  const int mixin = 3;

  for (size_t n_outputs = 1; n_outputs <= max_outputs; ++n_outputs)
  {
//    std::vector<uint64_t> outamounts;
    rct::keyV amount_keys;
    rct::keyV destinations;
    rct::key Sk, Pk;
    uint64_t available = 6000 + 7000;
    uint64_t amount;
    rct::ctkeyM mixRing(sc.size());

    //add output
    for (size_t i = 0; i < n_outputs; ++i)
    {
      amount = rct::randXmrAmount(available);
      outamounts.push_back(amount);
      amount_keys.push_back(rct::hash_to_scalar(rct::zero()));
      rct::skpkGen(Sk, Pk);
      destinations.push_back(Pk);
      available -= amount;
    }

    for (size_t i = 0; i < sc.size(); ++i)
    {
      for (size_t j = 0; j <= mixin; ++j)
      {
        if (j == 1)
          mixRing[i].push_back(pc[i]);
        else
          mixRing[i].push_back({rct::scalarmultBase(rct::skGen()), rct::scalarmultBase(rct::skGen())});
      }
    }

    rct::ctkeyV outSk;
    rct::rctSig s = rct::genRctSimple([] (const rct::key &a) -> rct::key {return rct::zero();}, sc, destinations, inamounts, outamounts, available, mixRing, amount_keys, NULL, NULL, index, outSk, rct::RangeProofPaddedBulletproof, hw::get_device("default"));
    assert(rct::verRctSimple(s, {}));
    for (size_t i = 0; i < n_outputs; ++i)
    {
      rct::key mask;
      rct::decodeRctSimple(s, amount_keys[i], i, mask, hw::get_device("default"), {});
      assert(mask == outSk[i].mask);
    }
  }
//
//
//
////  const crypto::hash tx_hash{0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
////                             0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
////                             0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
////                             0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
//
//  const rct::key mask{0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12,
//                      0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12,
//                      0x12, 0x11, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12,
//                      0x12, 0x11, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12};
//
//  rct::key tx_hash = rct::skGen();
//
//  std::vector<rct::xmr_amount> in_amounts;
//  std::vector<rct::xmr_amount> out_amounts;
//
//  rct::ctkeyV sc, pc;
//  rct::ctkey sctmp, pctmp;
//
//  rct::keyV destinations;
//  rct::keyV amount_keys;
//
//
//  //add fake input 3000
//  //the sc is secret data
//  //pc is public data
//  std::tie(sctmp, pctmp) = rct::ctskpkGen(3000);
//  sc.push_back(sctmp);
//  pc.push_back(pctmp);
//  in_amounts.push_back(3000);
//
//  //add fake input 3000
//  //the sc is secret data
//  //pc is public data
//  std::tie(sctmp, pctmp) = rct::ctskpkGen(3000);
//  sc.push_back(sctmp);
//  pc.push_back(pctmp);
//  in_amounts.push_back(3000);
//
//  //add output 5000
//  out_amounts.push_back(5000);
//  amount_keys.push_back(rct::hash_to_scalar(rct::zero()));
//  //add the corresponding destination pubkey
//  rct::key Sk, Pk;
//  skpkGen(Sk, Pk);
//  destinations.push_back(Pk);
//
//  //add output 999
//  out_amounts.push_back(999);
//  amount_keys.push_back(rct::hash_to_scalar(rct::zero()));
//  //add the corresponding destination pubkey
//  skpkGen(Sk, Pk);
//  destinations.push_back(Pk);
//
//  rct::xmr_amount txnfee = 1;
//
//  rct::rctSig s = genRctSimple(
//      tx_hash,
//      sc,
//      pc,
//      destinations,
//      in_amounts,
//      out_amounts,
//      amount_keys,
//      nullptr,
//      nullptr,
//      txnfee,
//      2,
//      hw::get_device("default"));
//
//  std::cerr << "Verification result: " << verRctSimple(s);

#endif
}

// convert hex string to string that has values based on that hex
// thankfully should automatically ignore null-terminator.
std::string h2b(const std::string& s)
{
  bool upper = true;
  std::string result;
  unsigned char val = 0;
  for (char c : s)
  {
    if (upper)
    {
      val = 0;
      if (c <= 'f' && c >= 'a')
      {
        val = ((c - 'a') + 10) << 4;
      }
      else
      {
        val = (c - '0') << 4;
      }
    }
    else
    {
      if (c <= 'f' && c >= 'a')
      {
        val |= (c - 'a') + 10;
      }
      else
      {
        val |= c - '0';
      }
      result += (char)val;
    }
    upper = !upper;
  }
  return result;
}

bool create_tx(tools::wallet2 *wallet) /*Analog of simple_wallet::transfer_main*/
{
  uint32_t priority = 0;
  priority = wallet->adjust_priority(priority);
  size_t fake_outs_count = 0;
  uint64_t adjusted_fake_outs_count = wallet->adjust_mixin(fake_outs_count);
  std::vector<uint8_t> extra;
  uint64_t locked_blocks = 0;

  std::vector<cryptonote::tx_destination_entry> dsts;
  size_t num_subaddresses = 0;

  cryptonote::tx_destination_entry de;
  de.addr   = wallet->get_account().get_keys().m_account_address;
  de.amount = 1000000000;
  de.token_id = 1;
  dsts.push_back(de);

  auto current_subaddress_account = 0;
  std::set<uint32_t> subaddr_indices;

  try
  {
    uint64_t bc_height, unlock_block = 0;
    std::string err;

    std::vector<tools::pending_tx> ptx_vector = wallet->create_transactions_2(
        dsts,
        fake_outs_count,
        0 /* unlock_time */,
        priority,
        extra,
        current_subaddress_account,
        subaddr_indices);

    if (ptx_vector.empty())
    {
      std::cout << "No outputs found, or daemon is not ready";
      return true;
    }

    // actually commit the transactions
    wallet->commit_tx(ptx_vector.front());
  }
  catch (const std::exception &e)
  {
    LOG_ERROR("unknown error");
  }
  catch (...)
  {
    LOG_ERROR("unknown error");
  }

  return true;
}

void test_token_transfer()
{
  //////////////////////////////////////////////////////////////////////////////////
  /// initialize blockchain
  //////////////////////////////////////////////////////////////////////////////////

  boost::filesystem::path path("./");
  std::string str_path = path.string();

  struct Storage {
    cryptonote::Blockchain     d_core_storage;
    cryptonote::tx_memory_pool d_mempool;

    Storage ()
    : d_core_storage(d_mempool)
    , d_mempool(d_core_storage) {}
  } s;

  cryptonote::BlockchainDB *db = new cryptonote::BlockchainLMDB();
  {
    db->open(str_path);
    bool res = s.d_core_storage.init(db, cryptonote::network_type::MAINNET);
  }

  cryptonote::account_base account;
  account.generate();
  cryptonote::account_public_address address = account.get_keys().m_account_address;


  //////////////////////////////////////////////////////////////////////////////////
  /// create block template
  //////////////////////////////////////////////////////////////////////////////////

  cryptonote::block           block;
  cryptonote::difficulty_type difficulty = 100000;
  uint64_t                    height = s.d_core_storage.get_current_blockchain_height();
  uint64_t                    expected_reward = 0;
  cryptonote::blobdata        ex_nonce{};

  {
    std::vector<cryptonote::transaction> txs;
    s.d_mempool.get_transactions(txs, false);
    for (auto& tx: txs) {
      cryptonote::transaction tx_data;
      size_t tx_weight;
      uint64_t fee;
      bool b1, b2, b3;
      s.d_mempool.take_tx(tx.hash, tx_data, tx_weight, fee, b1, b2, b3);
    }
    bool res = s.d_core_storage.create_block_template(block, address, difficulty, height, expected_reward, ex_nonce);
    std::cout << "miner tx: " << block.miner_tx.hash << std::endl;
  }


  //////////////////////////////////////////////////////////////////////////////////
  /// add new block to blockchain (for test purpose)
  //////////////////////////////////////////////////////////////////////////////////
  cryptonote::block_verification_context bvc = boost::value_initialized<cryptonote::block_verification_context>();
  bool res = s.d_core_storage.add_new_block(block, bvc);


  //////////////////////////////////////////////////////////////////////////////////
  /// create one time wallet
  //////////////////////////////////////////////////////////////////////////////////
  const cryptonote::network_type nettype = cryptonote::network_type::MAINNET;
  const uint64_t kdf_rounds = 1;
  const bool unattended = true;

  auto daemon_host = "localhost";
  auto daemon_port = get_config(nettype).RPC_DEFAULT_PORT;
  auto daemon_address = std::string("http://") + daemon_host + ":" + std::to_string(daemon_port);
  boost::optional<epee::net_utils::http::login> login{};
  boost::optional<bool> trusted_daemon = true;

  tools::wallet2 *wallet = new tools::wallet2(nettype, kdf_rounds, unattended);
  wallet->init(std::move(daemon_address), std::move(login), 0, false, *trusted_daemon);

  //////////////////////////////////////////////////////////////////////////////////
  /// add cutcoins to the wallet
  //////////////////////////////////////////////////////////////////////////////////
  {
//    std::vector<cryptonote::transaction> txs;
//    s.d_mempool.get_transactions(txs, false);

    cryptonote::transaction tx = block.miner_tx;

    cryptonote::block_complete_entry bce;
    bce.block = cryptonote::block_to_blob(block);
    for (const auto &tx_hash: block.tx_hashes) {
      cryptonote::blobdata txblob;
      bce.txs.push_back(txblob);
    }
    std::vector<cryptonote::block_complete_entry> bcev;
    bcev.push_back(bce);

    tools::wallet2::parsed_block pb;
    pb.block = block;
    pb.hash = block.hash;
//    pb.txes.push_back(block.miner_tx);

    std::vector<tools::wallet2::parsed_block> pbv;
    pbv.push_back(pb);


    uint64_t blocks_added;

//    wallet->process_parsed_blocks(
//        0,
//        bcev,
//        pbv,
//        blocks_added);
  }


  // add cutcoins to the account 10000000000

  std::vector<cryptonote::tx_destination_entry> dsts;
  cryptonote::tx_destination_entry self;
  self.addr = wallet->get_account().get_keys().m_account_address;
  self.amount = 10000000000;
  self.token_id = 0;
  dsts.push_back(self);
//  wallet->create_fake_transaction(dsts);

//  cryptonote::transaction tx;
//  tools::transfer_details td = wallet->get_transfer_details(0);
//  cryptonote::transaction_prefix tp = td.m_tx;
//
//  std::vector<cryptonote::tx_out> outs;
//
//  cryptonote::tx_out o;
//  o.token_id = 0;
//  o.amount = 10000000000;
//
//  outs.push_back(o);





  // add tokens to the account 1000

  // create tx v.3
  {
    bool r = create_tx(wallet);
  }



  s.d_core_storage.deinit();
  usleep(1000000);
}

// Helper function to generate genesis transaction
// https://monero.stackexchange.com/questions/8468/how-can-i-get-my-own-genesis-tx-to-use-in-my-monero-fork
void print_genesis_tx_hex(uint8_t nettype) {
  using namespace cryptonote;

  account_base miner_acc1;
  miner_acc1.generate();

  std::cout << "Gennerating miner wallet..." << std::endl;
  std::cout << "Miner account address:" << std::endl;
  std::cout << cryptonote::get_account_address_as_str((network_type)nettype, false, miner_acc1.get_keys().m_account_address);
  std::cout << std::endl << "Miner spend secret key:"  << std::endl;
  epee::to_hex::formatted(std::cout, epee::as_byte_span(miner_acc1.get_keys().m_spend_secret_key));
  std::cout << std::endl << "Miner view secret key:" << std::endl;
  epee::to_hex::formatted(std::cout, epee::as_byte_span(miner_acc1.get_keys().m_view_secret_key));
  std::cout << std::endl << std::endl;

//Create file with miner keys information
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  std::stringstream key_fine_name_ss;
  key_fine_name_ss << "./miner01_keys" << std::put_time(&tm, "%Y%m%d%H%M%S") << ".dat";
  std::string key_file_name = key_fine_name_ss.str();
  std::ofstream miner_key_file;
  miner_key_file.open (key_file_name);
  miner_key_file << "Miner account address:" << std::endl;
  miner_key_file << cryptonote::get_account_address_as_str((network_type)nettype, false, miner_acc1.get_keys().m_account_address);
  miner_key_file << std::endl<< "Miner spend secret key:"  << std::endl;
  epee::to_hex::formatted(miner_key_file, epee::as_byte_span(miner_acc1.get_keys().m_spend_secret_key));
  miner_key_file << std::endl << "Miner view secret key:" << std::endl;
  epee::to_hex::formatted(miner_key_file, epee::as_byte_span(miner_acc1.get_keys().m_view_secret_key));
  miner_key_file << std::endl << std::endl;
  miner_key_file.close();


//Prepare genesis_tx
  cryptonote::transaction tx_genesis;
  cryptonote::construct_miner_tx(0, 0, 10, 10, 0, miner_acc1.get_keys().m_account_address, tx_genesis);

  std::cout << "Object:" << std::endl;
  std::cout << obj_to_json_str(tx_genesis) << std::endl << std::endl;


  std::stringstream ss;
  binary_archive<true> ba(ss);
  ::serialization::serialize(ba, tx_genesis);
  std::string tx_hex = ss.str();
  std::cout << "Insert this line into your coin configuration file: " << std::endl;
  std::cout << "std::string const GENESIS_TX = \"" << epee::string_tools::buff_to_hex_nodelimer(tx_hex) << "\";" << std::endl;

  return;
}

int main ()
{
  //test_token_transfer();
  print_genesis_tx_hex(0);
  return 0;
}


//
//  const std::vector<size_t> sizes = {
//      1122
//  };
//
//  const std::vector<cryptonote::difficulty_type> diffs = {
//      4003674
//  };
//
//  const std::vector<uint64_t> coins =
//      {
//          3089241597253,
//          1000000000000
//      };
//
//  const std::vector<std::vector<std::string>> t_transactions =
//      {
//        {
//          "02000102000bb2da0b9060bdc906bc9409ccb1088ed804dd9003b025a544f501ea18a672fd7d57135405f121d3515ad63a77"
//          "3d03905a851e4a4346d2eaf6c3368b58020002888af452faa44cb0faa62cecee4ca514952af345167729304ae33deb551411"
//          "e90002d47d1ff1890e17dd977e0ebd9d1cadc75918c1fb2e062d49b6286d8f3421da9a8a010138ddad0bbf5ffb5cec4e4376"
//          "fe4e52a01e7e2ac678cc296653151687d113261d05e7c15ef8e435ab401d5babba805bdd46b39826a8670a9039aaa56d9a7a"
//          "554196c7100ddd898bb2a4e153eb08a86260e67a79efbe377d7b0b2710c55553add599cf377b8390b4ad58e32d0593d478f1"
//          "2813e4196c6ab4f8a79d91fac9b7d07aa90010a5d4e80000000300f63a7f971b40829bdbf7c93b4fd815f9c1534cf98a1e8f"
//          "5f0f8b5ae07807760bc54430f23a01f8177242ed81672505431dc9fb776fd2c931cb03f87cdae0a7055f8e6824f69019e991"
//          "a104d2906a24721a22a3edb8d4712ef852c7f971c5b700622ce03ccd9a4dde98bc42c6f796fe5b760ed06c0bae31c61c423c"
//          "005762b50ce5892dc1f19a5a7982af3a4287004352769dcdaa7b08e5d45584955d12cdfe3d15ed47305c8a66d4387334ba03"
//          "e3625bf2089704571bf9cf02fcf8030e75fdc9010000008c787156956f4ee6eff8bec1c5b29f4844ef976827634139cdd919"
//          "97adf6d81b73d0e15ee7b3d4bb085304bab1bdda8e0da9f9f26085ef9050a0b8665b33614a5c22f52788c11127eeb614f5cc"
//          "917001307fd2c7b89d543238fd499c23f5dd4fa3dd3fb7f99e6a3f4e19bdbb1244819939a216325d87db6d029a54f00b16f1"
//          "75421a0976cb197ce64cfc66872da5df5ad177fac1f678dcd3d57fe3634962410fb6d8da01f569e4f8c6ba63d8a2054bc661"
//          "1e31e86d9caa30c1e3c1db45c7b10907941ea48dfefa954ae04ca55f04f49430626c539e654d15c0e1a6421483ce2dddea66"
//          "a9370fe611e2b6f018a9001cb20c49cb52a1fd0f44c505dcc62c5bb891a44a5737e6c228ea443d92b226979269a3cef24496"
//          "2d39271c35386e594cf82d3d2e8fc87c421ad1e503472760d2320347c08f0e05b4a88c1f920d96e4a266db1ff98ce2adc110"
//          "87202267453632173f991c87f77922589b6ab8a972ce520f7673b7888a87f5bcf1bd5fb7d42db79ff7376d24dc9ae658c69e"
//          "d87c6d0a99ffffc4e0a8f6880a84292d97f9adcbcef9d240d734dc96e5707f4ccc2db9b5b4508f4c0782087ee68de48a069b"
//          "37b42ab85f95e3dac35ae469a94bb56f71ac12a73b9410c7ca8e7ee946cf15daf37f54003c7d964185c64b3591fd2addc883"
//          "0a24f21395653e6590425d3f5737d8b739a30292de2909459523a336fffc8c0c0809365d81c7a6f3a919ae9c5e4cfe5343a8"
//          "6f8ce07f11a8f536b41e7dd9a51c9e89410c33b324eeacc61326a0a944595ee0a0660c1d9bd45dd3415726582009d48f4427"
//          "62bf77a4611a783754d9a575fdfa96a323fbe1936c5d5a2765dac5125c16610d886e1ba0a20cc54a85fff28a9444e06ce887"
//          "bd6dff4aa48cbaf82c429a455d60ae88d395269c146ae083fea5800d1c055026868cf79796efed5c3e409b2a0ee901a02192"
//          "9b97afb6d36f2ee7100c0a4c07a5ffcdb20bc173d8e7459c63e8becc0505ef037b47aa52ede6130d6c35859a128baf1d7184"
//          "389fa99a4b55e9027cac0f9b1788988663a009c0310fe3782c727fc3f1b29de7a8cd2ece3eb67028e4a907386e2c7dc67972"
//          "4040fba2466de35ac2f76c7420bf05f28824857fa0e76f35082f2687d6066f11edec430b3a5e90a07d5f592ab27962699c26"
//          "baa783ad93a707d92aecfebd45f1b19dac3f79091869044e79b820202a25708e389b8d42f40e054f85474d12a6840b8c6610"
//          "9cb243cd93a2b7151621e5608ad5bb9203016d040d1311fa7212ac356012fd1d8a74ed65ea37abf55bf81844b5c4b3336cb9"
//          "2d860519312c6b652d619675d636fcbfa171c979becdb8205249691f5e110383b9dd084d795398d10e457ca725020d6a04b2"
//          "bd1b291da815d612274f3549964a5d7a0501d1d57fdcf093a54baa1a319e9d3598b4b3b48101289e2e39e74ba591221206d9"
//          "67fb350c53b4e3faf2ce6c7109ef6d444d0f09b0e71c3b61213ab90293e70a6c8155815d0d231928558659d0b2acfefb01a2"
//          "2cf8902a885e402275ea90a405b840dc39fd9e37bae370c44e160a8f065cfb6f47375e54f9b3bc6d11fa6ca8057041e7bc80"
//          "b6fd0a073ca250647994aa09662c2827a82417a1243439472cbd0229b4d7df8daae147e874b251c48e9aff2dfe1bae139d36"
//          "e850ed1833c2165408276f78f6c4e2ab7a640938a02e61093ca494c5706b4629621b92e18242269d01c4a4b368d74e28d283"
//          "52bdd65f53ba4df0c989e46d58ed026efe27e7465eac0dd3ee1d9c3b440d3226d97a8f4a3f90be43575d749b35147f7cdb6c"
//          "b691b28204fa917ebc3f1ed40f4e20cce68ba5f819987f1df540dc3ac564f41a59746f2c0442fff61de939a559f0e174e474"
//          "a84afb0789e0543735e19bb193f419b6e35804939e439c20f16e8bd1690374dc80719a7337fef2dbdfc834c19ac6ee020eb2"
//          "016511fd6e72490ab3c657d4baa17786c8982e2ef7ac7bc0f7b8c38a866f444c074de5d516a8009f5c088d0a612fe2d1f7f1"
//          "5cd951b74de3513bd13cbc2d658a0659ee236902a3b89b07218dd2aaa7da1eead0e2e08162ec61d4fb71d9aecd9e00926429"
//          "2e033fe78a7824c85b45fabc85b44a45ca2682f392e205357a1b18e2e5"
//        }
//      };
//

//
//  std::vector< std::vector<cryptonote::transaction> > txs;
//  for (auto& i : t_transactions) {
//    std::vector<cryptonote::transaction> ts;
//    for (auto& j : i)
//    {
//      cryptonote::transaction tx;
//      cryptonote::blobdata bd = h2b(j);
//      parse_and_validate_tx_from_blob(bd, tx);
//      ts.push_back(tx);
//    }
//    txs.push_back(ts);
//  }

//// Prepare i-th block
//// This one is 100000-th from Cutcoin blockchain
//const std::string block_blob = {
//    "0a0aa9f3e3e60544bb5fcf27bc52c0152525ae5e30501ffd4b37710afff0c722d19d2c6f0300000000000002dc8d0601ffa08d0604c5f6997302130684162775e653bdf1137dfdea2f4e53351cc4adc994911f8849c9d84cb37680b4c4c321024103de70a22db2bfc9098cc140308a88a09838045c5ee93d521062b5288ba8ca80c0fc82aa02024a72a7970fe3ac192cb4e233871fcb3f3a5e3e18fe45f6df462c0aa83661a4ec80e0bcefa757026b8cb71a1a87c97684af7336add75d9556f44f4df204cacc5aed93d3c588b5a42101700213781145652eb2300fe2d554c79addd0014f12fee96bc75efecfc3cd0d110001df4ad4b015a5ebc95b616e34f75b73d37c5f69dc57245d45de144597ebd897f0"
//};
//
//cryptonote::block           block;
//cryptonote::difficulty_type difficulty = 100000;
//uint64_t                    height = s.d_core_storage.get_current_blockchain_height();
//uint64_t                    expected_reward = 0;
//cryptonote::blobdata        ex_nonce{};
//
//cryptonote::blobdata bd = h2b(block_blob);
////  cryptonote::parse_and_validate_block_from_blob(bd, block);
//
//
//// add transaction to the pool
////  s.d_mempool.add_tx(transaction &tx, tx_verification_context& tvc, bool kept_by_block, bool relayed, bool do_not_relay, uint8_t version);