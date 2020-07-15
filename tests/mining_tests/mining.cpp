// Copyright (c) 2018-2020, CUT coin
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

#include "mining/atan.h"
#include "mining/miningutil.h"

#include "gtest/gtest.h"

#include <limits>

TEST(atan, specific_points)
{
  // Concerns: Fixed point implementation must match the standard library implementation
  // with certain accuracy.

  double floating_point_accuracy = 1.0e-2;
  int64_t base = 10;

  int64_t p_arguments[] = {0, 1, 5, 10, 50, 100, 1000, 0x1ll << 32, 0x1ll << 62};
  for (const auto argument: p_arguments) {
    double floating_point_atan = atan2(argument, base) / M_PI;
    double fixed_point_atan = ((double)mining::atan2c(argument, base)) / mining::r_scaling_factor;
//    std::cout << floating_point_atan << " " << fixed_point_atan << std::endl;
    EXPECT_LE(std::abs(floating_point_atan - fixed_point_atan),
              floating_point_accuracy * floating_point_atan);
  }
}

TEST(atan, signum)
{
  EXPECT_EQ(1, mining::sign(10));
  EXPECT_EQ(-1, mining::sign(-10));
}


TEST(atan, binary_search)
{
  // Concerns: Fixed point implementation of the binary search for smooth monotonic functions.
  // Examine specific points, diapason coverage, diapason bound points.

  uint32_t l_bound = 0;
  uint32_t u_bound = 10;
  for (uint32_t i = l_bound; i <= u_bound; ++i) {
    num::u128_t res = mining::binary_search(l_bound, u_bound, i, [](uint32_t x) { return num::u128_t(x); });
    EXPECT_EQ(i, res.val[0]);
  }
}
