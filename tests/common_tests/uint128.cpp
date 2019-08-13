// Copyright (c) 2018-2019, CUT coin
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


#include "common/uint128.hpp"

#include "gtest/gtest.h"

using namespace num;

// Test value-semantic type

const size_t zero = 0ull;
const size_t one  = 1ull;
const size_t max  = std::numeric_limits<uint64_t>::max();

TEST(uint128, creaters)
{
  {
    // creation from uint64_t
    u128_t a(zero);
    EXPECT_EQ(a, zero);

    u128_t b(one);
    EXPECT_EQ(b, one);

    u128_t c(max);
    EXPECT_EQ(c, max);
  }

  {
    // creation from 2 uint64_t
    u128_t a(zero, zero);
    EXPECT_EQ(a, zero);

    u128_t b(one, zero);
    EXPECT_EQ(b, one);

    u128_t c(max, zero);
    EXPECT_EQ(c, max);
  }

  {
    // creation from another uint128_t
    u128_t a(zero);
    u128_t b(one);
    u128_t c(max);

    u128_t _a(a);
    EXPECT_EQ(_a, zero);

    u128_t _b(b);
    EXPECT_EQ(_b, one);

    u128_t _c(c);
    EXPECT_EQ(_c, max);
  }

  {
    // creation from string
    u128_t as("0");
    EXPECT_EQ(as, zero);

    u128_t bs("1");
    EXPECT_EQ(bs, one);

    u128_t cs("18446744073709551615");
    EXPECT_EQ(cs, max);
  }

  {
    // wrong string literals
    EXPECT_ANY_THROW(u128_t{"abc"});
    EXPECT_ANY_THROW(u128_t{"111111111111111111111111111111111111111111111"});
  }

  {
    // check operator assignment
    u128_t a(zero);
    u128_t b(one);
    u128_t c(max);

    u128_t a_dot = a;
    EXPECT_EQ(a_dot, zero);

    u128_t b_dot = b;
    EXPECT_EQ(b_dot, one);

    u128_t c_dot = c;
    EXPECT_EQ(c_dot, max);
  }

  {
    // check equality operator
    u128_t a(zero);
    u128_t b(one);
    u128_t c(max);
    u128_t d(one, one);
    u128_t e("18446744073709551617");
    u128_t f(max, max);

    u128_t _a = a;
    u128_t _b = b;
    u128_t _c = c;
    u128_t _d = d;
    u128_t _e = e;
    u128_t _f = f;

    u128_t __a = a;
    u128_t __b = b;
    u128_t __c = c;
    u128_t __d = d;
    u128_t __e = e;
    u128_t __f = f;

    EXPECT_EQ(_a, __a);
    EXPECT_EQ(_b, __b);
    EXPECT_EQ(_c, __c);
    EXPECT_EQ(_d, __d);
    EXPECT_EQ(_e, __e);
    EXPECT_EQ(_f, __f);

    EXPECT_EQ(_d, __e);
  }
}

TEST(uint128, DISABLED_unary_operators)
{
  // Concerns: conversion to bool, increment, decrement.

  {
    // conversion to bool
    u128_t a(zero);
    EXPECT_FALSE(!!a);

    u128_t b(one);
    EXPECT_TRUE(!!b);

    u128_t c(max);
    EXPECT_TRUE(!!c);
  }

  {
    // increment
    u128_t a(zero);
    ++a;
    EXPECT_EQ(a, one);

    u128_t b(one);
    ++b;
    EXPECT_EQ(b, 2ull);

    u128_t c(max);
    ++c;
    EXPECT_EQ(c, u128_t(zero, one));

    u128_t d(zero);
    u128_t e = ++d;
    EXPECT_EQ(e, one);

    u128_t f(max, max);
    auto func = [&]() {++f;};
    EXPECT_ANY_THROW(func());
  }

  {
    // decrement
    u128_t a(one);
    --a;
    EXPECT_EQ(a, zero);

    u128_t b(2ull);
    --b;
    EXPECT_EQ(b, one);

    u128_t c(zero, one);
    --c;
    EXPECT_EQ(c, u128_t(max, zero));

    u128_t d(one);
    u128_t e = --d;
    EXPECT_EQ(e, zero);

    u128_t f(zero);
    auto func = [&]() {--f;};
    EXPECT_ANY_THROW(func());
  }
}

TEST(uint128, stream_operator)
{
  // Concerns: stream operator.
}

TEST(uint128, arithmetic_operators)
{
  // Concerns: arithmetic operations.

  {
    // addition
    u128_t a(zero);
    u128_t b(one);
    u128_t c(max);
    u128_t d(zero, one);
    u128_t e(max, max);

    EXPECT_EQ(a + a, a);
    EXPECT_EQ(a + b, b);
    EXPECT_EQ(b + a, b);
    EXPECT_EQ(a + c, c);
    EXPECT_EQ(c + a, c);
    EXPECT_EQ(b + c, d);
    EXPECT_EQ(c + b, d);
    EXPECT_EQ(a + e, e);
    EXPECT_EQ(e + a, e);
    EXPECT_ANY_THROW(e + b);
    EXPECT_ANY_THROW(b + e);
    EXPECT_ANY_THROW(e + c);
    EXPECT_ANY_THROW(c + e);
  }

  {
    // subtraction
    u128_t a(zero);
    u128_t b(one);
    u128_t c(max);
    u128_t d(zero, one);
    u128_t e(max, max);

    EXPECT_EQ(a - a, a);
    EXPECT_EQ(b - a, b);
    EXPECT_EQ(b - b, a);
    EXPECT_EQ(c - a, c);
    EXPECT_EQ(c - b, u128_t(max - one, zero));
    EXPECT_EQ(c - c, a);
    EXPECT_EQ(d - a, d);
    EXPECT_EQ(d - b, c);
    EXPECT_EQ(d - c, b);
    EXPECT_EQ(e - a, e);
    EXPECT_EQ(e - b, u128_t(max - one, max));
    EXPECT_EQ(e - c, u128_t(zero, max));
    EXPECT_EQ(e - e, zero);
    EXPECT_ANY_THROW(a - b);
    EXPECT_ANY_THROW(a - c);
    EXPECT_ANY_THROW(a - d);
    EXPECT_ANY_THROW(a - e);
    EXPECT_ANY_THROW(b - c);
    EXPECT_ANY_THROW(b - d);
    EXPECT_ANY_THROW(b - e);
    EXPECT_ANY_THROW(c - d);
    EXPECT_ANY_THROW(c - e);
    EXPECT_ANY_THROW(d - e);
  }
}
