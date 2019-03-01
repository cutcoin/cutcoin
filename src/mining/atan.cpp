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

#include "atan.h"

#include <cstdlib>
#include <cstdint>

namespace mining {

int64_t atan2c(int64_t y, int64_t x) {
  if (y == 0) {
    return (x >= 0 ? 0 : r_scaling_factor);
  }

  int64_t phi = 0;
  if (x <= y) {
    int64_t t = y - x;
    x = x + y;
    y = t;
    phi += 1;
  }

  phi *= (r_scaling_factor >> 2);

  if (x < 0x10000) {
    x *= 0x1000;
    y *= 0x1000;
  }

  const uint16_t list[] = {
      0x4000, 0x25C8, 0x13F6, 0x0A22, 0x0516, 0x028C, 0x0146, 0x00A3,
      0x0051, 0x0029, 0x0014, 0x000A, 0x0005, 0x0003, 0x0001, 0x0001
  };

  int64_t i, tmp, dphi = 0;
  for (i = 1; i < 12; ++i) {
    if (y >= 0) {
      tmp = x + (y >> i);
      y = y - (x >> i);
      x = tmp;
      dphi += list[i];
    } else {
      tmp = x - (y >> i);
      y = y + (x >> i);
      x = tmp;
      dphi -= list[i];
    }
  }

  return phi + dphi / 4;
}

}