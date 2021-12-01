
#include "crypto/hash.h"
#include "cryptonote_basic/account.h"
#include "cryptonote_basic/cryptonote_basic.h"
#include "cryptonote_basic/cryptonote_format_utils.h"
#include "cryptonote_basic/dex.h"
#include "cryptonote_core/cryptonote_tx_utils.h"
#include "cryptonote_core/tx_source_entry.h"
#include "serialization/json_object.h"

#include <boost/optional/optional.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <gtest/gtest.h>
#include <rapidjson/document.h>

#include <vector>


namespace
{
    cryptonote::transaction
    make_miner_transaction(cryptonote::account_public_address const& to)
    {
        cryptonote::transaction tx{};
        if (!cryptonote::construct_miner_tx(0, 0, 5000, 500, 500, to, tx))
            throw std::runtime_error{"transaction construction error"};

        crypto::hash id{0};
        if (!cryptonote::get_transaction_hash(tx, id))
            throw std::runtime_error{"could not get transaction hash"};

        return tx;
    }

    cryptonote::transaction
    make_transaction(
        cryptonote::account_keys const& from,
        std::vector<cryptonote::transaction> const& sources,
        std::vector<cryptonote::account_public_address> const& destinations,
        bool rct,
        bool bulletproof)
    {
        std::uint64_t source_amount = 0;
        cryptonote::TxSources actual_sources;
        for (auto const& source : sources)
        {
            std::vector<cryptonote::tx_extra_field> extra_fields;
            if (!cryptonote::parse_tx_extra(source.extra, extra_fields))
                throw std::runtime_error{"invalid transaction"};

            cryptonote::tx_extra_pub_key key_field{};
            if (!cryptonote::find_tx_extra_field_by_type(extra_fields, key_field))
                throw std::runtime_error{"invalid transaction"};

            for (auto const& input : boost::adaptors::index(source.vout))
            {
                source_amount += input.value().amount;
                auto const& key = boost::get<cryptonote::txout_to_key>(input.value().target);

                cryptonote::tx_source_entry se;
                se.outputs = {};
                se.real_output = 0;
                se.real_out_tx_key = key_field.pub_key;
                se.real_out_additional_tx_keys = {};
                se.real_output_in_tx_index = std::size_t(input.index());
                se.amount = input.value().amount;
                se.o_type = cryptonote::SourceType::wallet;
                se.rct = rct;
                se.mask = rct::identity();
                se.multisig_kLRki = {};
                se.token_id = cryptonote::CUTCOIN_ID;
                actual_sources.emplace_back(se);

                for (unsigned ring = 0; ring < 10; ++ring)
                    actual_sources.back().push_output(input.index(), key.key, input.value().amount);
            }
        }

        std::vector<cryptonote::tx_destination_entry> to;
        for (auto const& destination : destinations)
            to.emplace_back(0, (source_amount / destinations.size()), destination, false, false, false, false);

        cryptonote::transaction tx{};

        crypto::secret_key tx_key{};
        std::vector<crypto::secret_key> extra_keys{};

        std::unordered_map<crypto::public_key, cryptonote::subaddress_index> subaddresses;
        subaddresses[from.m_account_address.m_spend_public_key] = {0,0};

        cryptonote::TxConstructionContext context;
        context.d_sender_account_keys = from;
        context.d_subaddresses        = subaddresses;
        context.d_sources             = actual_sources;
        context.d_destinations        = to;
//        context.d_change_addr         = boost::none;
        context.d_tx_key              = tx_key;
        context.d_additional_tx_keys  = extra_keys;
        context.d_range_proof_type    = rct::RangeProofBulletproof;
        if (!cryptonote::construct_tx_and_get_tx_key(context, tx))
            throw std::runtime_error{"transaction construction error"};

        return tx;
    }
} // anonymous

TEST(JsonSerialization, MinerTransaction)
{
    cryptonote::account_base acct;
    acct.generate();
    const auto miner_tx = make_miner_transaction(acct.get_keys().m_account_address);

    crypto::hash tx_hash{};
    ASSERT_TRUE(cryptonote::get_transaction_hash(miner_tx, tx_hash));

    rapidjson::Document doc;
    cryptonote::json::toJsonValue(doc, miner_tx, doc);

    cryptonote::transaction miner_tx_copy;
    cryptonote::json::fromJsonValue(doc, miner_tx_copy);

    crypto::hash tx_copy_hash{};
    ASSERT_TRUE(cryptonote::get_transaction_hash(miner_tx_copy, tx_copy_hash));
    EXPECT_EQ(tx_hash, tx_copy_hash);

    cryptonote::blobdata tx_bytes{};
    cryptonote::blobdata tx_copy_bytes{};

    ASSERT_TRUE(cryptonote::t_serializable_object_to_blob(miner_tx, tx_bytes));
    ASSERT_TRUE(cryptonote::t_serializable_object_to_blob(miner_tx_copy, tx_copy_bytes));

    EXPECT_EQ(tx_bytes, tx_copy_bytes);
}

TEST(JsonSerialization, RegularTransaction)
{
    cryptonote::account_base acct1;
    acct1.generate();

    cryptonote::account_base acct2;
    acct2.generate();

    const auto miner_tx = make_miner_transaction(acct1.get_keys().m_account_address);
    const auto tx = make_transaction(
        acct1.get_keys(), {miner_tx}, {acct2.get_keys().m_account_address}, false, false
    );

    crypto::hash tx_hash{};
    ASSERT_TRUE(cryptonote::get_transaction_hash(tx, tx_hash));

    rapidjson::Document doc;
    cryptonote::json::toJsonValue(doc, tx, doc);

    cryptonote::transaction tx_copy;
    cryptonote::json::fromJsonValue(doc, tx_copy);

    crypto::hash tx_copy_hash{};
    ASSERT_TRUE(cryptonote::get_transaction_hash(tx_copy, tx_copy_hash));
    EXPECT_EQ(tx_hash, tx_copy_hash);

    cryptonote::blobdata tx_bytes{};
    cryptonote::blobdata tx_copy_bytes{};

    ASSERT_TRUE(cryptonote::t_serializable_object_to_blob(tx, tx_bytes));
    ASSERT_TRUE(cryptonote::t_serializable_object_to_blob(tx_copy, tx_copy_bytes));

    EXPECT_EQ(tx_bytes, tx_copy_bytes);
}

TEST(JsonSerialization, DISABLED_RingctTransaction)
{
    cryptonote::account_base acct1;
    acct1.generate();

    cryptonote::account_base acct2;
    acct2.generate();

    const auto miner_tx = make_miner_transaction(acct1.get_keys().m_account_address);
    const auto tx = make_transaction(
        acct1.get_keys(), {miner_tx}, {acct2.get_keys().m_account_address}, true, false
    );

    crypto::hash tx_hash{};
    ASSERT_TRUE(cryptonote::get_transaction_hash(tx, tx_hash));

    rapidjson::Document doc;
    cryptonote::json::toJsonValue(doc, tx, doc);

    cryptonote::transaction tx_copy;
    cryptonote::json::fromJsonValue(doc, tx_copy);

    crypto::hash tx_copy_hash{};
    ASSERT_TRUE(cryptonote::get_transaction_hash(tx_copy, tx_copy_hash));
    EXPECT_EQ(tx_hash, tx_copy_hash);

    cryptonote::blobdata tx_bytes{};
    cryptonote::blobdata tx_copy_bytes{};

    ASSERT_TRUE(cryptonote::t_serializable_object_to_blob(tx, tx_bytes));
    ASSERT_TRUE(cryptonote::t_serializable_object_to_blob(tx_copy, tx_copy_bytes));

    EXPECT_EQ(tx_bytes, tx_copy_bytes);
}

TEST(JsonSerialization, DISABLED_BulletproofTransaction)
{
    cryptonote::account_base acct1;
    acct1.generate();

    cryptonote::account_base acct2;
    acct2.generate();

    const auto miner_tx = make_miner_transaction(acct1.get_keys().m_account_address);
    const auto tx = make_transaction(
        acct1.get_keys(), {miner_tx}, {acct2.get_keys().m_account_address}, true, true
    );

    crypto::hash tx_hash{};
    ASSERT_TRUE(cryptonote::get_transaction_hash(tx, tx_hash));

    rapidjson::Document doc;
    cryptonote::json::toJsonValue(doc, tx, doc);

    cryptonote::transaction tx_copy;
    cryptonote::json::fromJsonValue(doc, tx_copy);

    crypto::hash tx_copy_hash{};
    ASSERT_TRUE(cryptonote::get_transaction_hash(tx_copy, tx_copy_hash));
    EXPECT_EQ(tx_hash, tx_copy_hash);

    cryptonote::blobdata tx_bytes{};
    cryptonote::blobdata tx_copy_bytes{};

    ASSERT_TRUE(cryptonote::t_serializable_object_to_blob(tx, tx_bytes));
    ASSERT_TRUE(cryptonote::t_serializable_object_to_blob(tx_copy, tx_copy_bytes));

    EXPECT_EQ(tx_bytes, tx_copy_bytes);
}

