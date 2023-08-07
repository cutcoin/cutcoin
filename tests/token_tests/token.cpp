// Copyright (c) 2018-2022, CUT coin
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

#include "cryptonote_basic/token.h"

#include "gtest/gtest.h"

#include <stdexcept>

using namespace cryptonote;

TEST(token, breathing)
{
  const std::string token_name{};
  try {
    token_name_to_id(token_name);
    EXPECT_TRUE(false);
  } catch (std::runtime_error &e) {
    EXPECT_TRUE(true);
  }
}

TEST(token, round_conversion)
{
    {
        const std::string token_name = "SHITCOIN";
        TokenId a = token_name_to_id(token_name);
        std::string b = token_id_to_name(a);
        EXPECT_EQ(b, token_name);
    }

    {
        const std::string token_name = "HITCOIN";
        TokenId a = token_name_to_id(token_name);
        std::string b = token_id_to_name(a);
        EXPECT_EQ(b, token_name);
    }
}

TEST(token, name_to_id)
{
    const std::string token_name = "A";
    TokenId a = token_name_to_id(token_name);
    TokenId b = 65ull << 56ull;
    EXPECT_EQ(a, b);
}

TEST(token, id_to_name_zero_end)
{
    TokenId a[2];
    char *ptr1 = (char *)&a;
    for (size_t i = 0; i < sizeof(TokenId); ++i)
    {
        ptr1[i] = 'A';
    }
    a[1] = 0;

    EXPECT_EQ(token_id_to_name(a[0]), std::string(ptr1, sizeof(TokenId)));
}

TEST(token, id_to_name_0xff_end)
{
    TokenId b[2];
    char *ptr2 = (char *)&b;
    for (size_t i = 0; i < sizeof(TokenId); ++i)
    {
        ptr2[i] = 'A';
    }
    b[1] = 0xffffffffffffffff;

    EXPECT_EQ(token_id_to_name(b[0]), std::string(ptr2, sizeof(TokenId)));
}

TEST(token, cutcoin_name)
{
    EXPECT_TRUE(validate_token_name("SHITCOIN"));
    EXPECT_FALSE(validate_token_name(""));
    EXPECT_FALSE(validate_token_name("CUTCOINLONG"));
    EXPECT_FALSE(validate_token_name("cut_coin"));
    EXPECT_FALSE(validate_token_name("CUT"));
    EXPECT_FALSE(validate_token_name("CUTCOIN"));
    EXPECT_FALSE(validate_token_name("CUTC0IN"));
}

TEST(token, lp_round_conversion)
{
  {
    const std::string token_name = "lpSHITCOIN";
    TokenId a = token_name_to_id(token_name);
    std::string b = token_id_to_name(a);
    EXPECT_EQ(b, token_name);
  }

  {
    const std::string token_name = "lpHITCOIN";
    TokenId a = token_name_to_id(token_name);
    std::string b = token_id_to_name(a);
    EXPECT_EQ(b, token_name);
  }
}

TEST(token, lp_name_to_id)
{
  const std::string token_name = "lpA";
  TokenId a = token_name_to_id(token_name);
  TokenId b = (65ull | 0x80ull) << 56ull;
  EXPECT_EQ(a, b);
}

TEST(token, validate_lptoken_name)
{
  EXPECT_TRUE(validate_lptoken_name("lpSHITCOIN"));
  EXPECT_FALSE(validate_lptoken_name("SHITCOIN"));
  EXPECT_FALSE(validate_lptoken_name(""));
  EXPECT_FALSE(validate_lptoken_name("lp"));
  EXPECT_TRUE(validate_lptoken_name("lpA"));
  EXPECT_FALSE(validate_lptoken_name("lpa"));
  EXPECT_FALSE(validate_lptoken_name("lpCUTCOINLONG"));
  EXPECT_FALSE(validate_lptoken_name("lpcut_coin"));
  EXPECT_FALSE(validate_lptoken_name("lpCUT"));
  EXPECT_FALSE(validate_lptoken_name("lpCUTCOIN"));
  EXPECT_FALSE(validate_lptoken_name("lpCUTC0IN"));
}