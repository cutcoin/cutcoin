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

#include "uint128.hpp"

namespace num {

#if defined(__x86_64__)

static inline void long_mul(uint64_t a, uint64_t b, uint64_t &rh, uint64_t &rl) {
  asm volatile(
  "mul %%rdx\n"
  : "+d" (a), "+a" (b)
  :
  : "cc");
  rh=a;
  rl=b;
}

static inline uint64_t long_div(uint64_t h, uint64_t l, uint64_t d) {
  asm volatile(
  "divq %2\n"
  : "+d" (h), "+a" (l)
  : "rm" (d)
  : "cc"
  );
  return l;
}

static inline void long_div64(uint64_t h, uint64_t l, uint64_t d, uint64_t &q, uint64_t &r) {
  asm volatile(
  "divq %2\n"
  : "+d" (h), "+a" (l)
  : "rm" (d)
  : "cc"
  );
  q=l;
  r=h;
}

static u128_t safe_add(const u128_t &a, const u128_t &b) {
  u128_t ret;
  uint32_t overflow;
  asm volatile(
  "mov %3, %%rax\n"
  "add %4, %%rax\n"
  "mov %%rax, %0\n"
  "mov %5, %%rax\n"
  "adc %6, %%rax\n"
  "mov %%rax, %1\n"
  "rcl $1, %%eax\n"
  : "=rm" (ret.val[0]), "=rm" (ret.val[1]), "=a" (overflow)
  : "rm" (a.val[0]), "rm" (b.val[0]), "rm" (a.val[1]), "rm" (b.val[1])
  : "cc"
  );
  if (overflow&1)
    throw std::runtime_error{"Addition overflow"};
  return ret;
}

static u128_t safe_sub(const u128_t &a, const u128_t &b) {
  u128_t ret;
  uint32_t overflow;
  asm volatile(
  "mov %3, %%rax\n"
  "sub %4, %%rax\n"
  "mov %%rax, %0\n"
  "mov %5, %%rax\n"
  "sbb %6, %%rax\n"
  "mov %%rax, %1\n"
  "rcl $1, %%eax\n"
  : "=rm" (ret.val[0]), "=rm" (ret.val[1]), "=a" (overflow)
  : "rm" (a.val[0]), "rm" (b.val[0]), "rm" (a.val[1]), "rm" (b.val[1])
  : "cc"
  );
  if (overflow&1)
    throw std::runtime_error{"Subtraction overflow"};
  return ret;
}

static inline u128_t unsafe_sub(const u128_t &a, const u128_t &b) {
  u128_t ret;
  asm volatile(
  "mov %2, %%rax\n"
  "sub %3, %%rax\n"
  "mov %%rax, %0\n"
  "mov %4, %%rax\n"
  "sbb %5, %%rax\n"
  "mov %%rax, %1\n"
  : "=rm" (ret.val[0]), "=rm" (ret.val[1])
  : "rm" (a.val[0]), "rm" (b.val[0]), "rm" (a.val[1]), "rm" (b.val[1])
  : "cc", "rax"
  );
  return ret;
}

#else

// The following is copied from src/cryptonote_basic/difficulty.cpp
static void long_mul(uint64_t a, uint64_t b, uint64_t &high, uint64_t &low) {
  uint64_t aLow = a & 0xFFFFFFFF;
  uint64_t aHigh = a >> 32;
  uint64_t bLow = b & 0xFFFFFFFF;
  uint64_t bHigh = b >> 32;

  uint64_t res = aLow * bLow;
  uint64_t lowRes1 = res & 0xFFFFFFFF;
  uint64_t carry = res >> 32;

  res = aHigh * bLow + carry;
  uint64_t highResHigh1 = res >> 32;
  uint64_t highResLow1 = res & 0xFFFFFFFF;

  res = aLow * bHigh;
  uint64_t lowRes2 = res & 0xFFFFFFFF;
  carry = res >> 32;

  res = aHigh * bHigh + carry;
  uint64_t highResHigh2 = res >> 32;
  uint64_t highResLow2 = res & 0xFFFFFFFF;

  //Addition

  uint64_t r = highResLow1 + lowRes2;
  carry = r >> 32;
  low = (r << 32) | lowRes1;
  r = highResHigh1 + highResLow2 + carry;
  uint64_t d3 = r & 0xFFFFFFFF;
  carry = r >> 32;
  r = highResHigh2 + carry;
  high = d3 | (r << 32);
}

static void long_div64(uint64_t h, uint64_t l, uint64_t d, uint64_t &q, uint64_t &r) {
  if (h) {
    uint64_t c;
    uint32_t i;
    if (h>=d)
      throw std::runtime_error{"Division overflow"};
    q=0;
    for (i=0; i<64; i++) {
      c=h>>63;
      h=(h<<1)+(l>>63);
      l<<=1;
      q<<=1;
      if (c || h>=d) {
        q|=1;
        h-=d;
      }
    }
    r=h;
  } else {
    q=l/d;
    r=l%d;
  }
}

static inline uint64_t long_div(uint64_t h, uint64_t l, uint64_t d) {
  uint64_t q, r;
  long_div64(h, l, d, q, r);
  return q;
}

static bool add64(uint64_t a, uint64_t b, uint64_t &r) {
  r=a+b;
  return r<a;
}

static u128_t safe_add(const u128_t &a, const u128_t &b) {
  u128_t ret;
  uint64_t ah;
  bool overflow;
  if (add64(a.val[0], b.val[0], ret.val[0])) {
    overflow=add64(a.val[1], 1, ah);
  } else {
    overflow=false;
    ah=a.val[1];
  }
  overflow=overflow || add64(ah, b.val[1], ret.val[1]);
  if (overflow)
    throw std::runtime_error{"Addition overflow"};
  return ret;
}

static bool sub64(uint64_t a, uint64_t b, uint64_t &r) {
  r=a-b;
  return b>a;
}

static u128_t safe_sub(const u128_t &a, const u128_t &b) {
  u128_t ret;
  uint64_t ah;
  bool overflow;
  if (sub64(a.val[0], b.val[0], ret.val[0])) {
    overflow=sub64(a.val[1], 1, ah);
  } else {
    overflow=false;
    ah=a.val[1];
  }
  overflow=overflow || sub64(ah, b.val[1], ret.val[1]);
  if (overflow)
    throw std::runtime_error{"Subtraction overflow"};
  return ret;
}

static inline u128_t unsafe_sub(const u128_t &a, const u128_t &b) {
  u128_t ret;
  uint64_t ah;
  if (sub64(a.val[0], b.val[0], ret.val[0]))
    sub64(a.val[1], 1, ah);
  else
    ah=a.val[1];
  sub64(ah, b.val[1], ret.val[1]);
  return ret;
}

#endif

static inline u128_t unsafe_mul(const u128_t &a, uint64_t b) {
  u128_t ret;
  if (a.val[1]) {
    uint64_t h, l;
    long_mul(a.val[0], b, ret.val[1], ret.val[0]);
    long_mul(a.val[1], b, h, l);
    ret.val[1]+=l;
  } else {
    long_mul(a.val[0], b, ret.val[1], ret.val[0]);
  }
  return ret;
}

void u128_t::set_cstring(const char *s) {
  uint64_t mh, ml;
  if (s[0]<'0' || s[0]>'9')
    throw std::runtime_error{"Error parsing number"};
  val[0]=s[0]-'0';
  val[1]=0;
  s++;
  while (s[0]>='0' && s[0]<='9') {
    long_mul(val[1], 10, mh, ml);
    if (mh)
      goto overflow;
    val[1]=ml;
    long_mul(val[0], 10, mh, ml);
    val[0]=ml;
    val[1]+=mh;
    if (val[1]<mh)
      goto overflow;
    ml=s[0]-'0';
    val[0]+=ml;
    if (val[0]<ml)
      if (++val[1]==0)
        goto overflow;
    s++;
  }
  return;
  overflow:
  throw std::runtime_error{"Parsing overflow"};
}

std::string u128_t::get_string() const {
  char str[48], *ptr;
  u128_t v=*this;
  uint32_t rem;
  ptr=str+sizeof(str);
  *--ptr=0;
  do {
    div128byconst<10>(v, v, rem);
    *--ptr='0'+rem;
  } while (v>0);
  return std::string(ptr);
}

u128_t& u128_t::operator++() {
  if (!++val[0] && !++val[1])
    throw std::runtime_error{"Addition overflow"};
  return *this;
}

u128_t& u128_t::operator--() {
  if (!val[0]-- && !val[1]--)
    throw std::runtime_error{"Subtraction overflow"};
  return *this;
}


std::ostream &operator <<(std::ostream &out, const u128_t &f) {
  out << f.get_string();
  return out;
}

u128_t operator +(const u128_t &a, const u128_t &b) {
  return safe_add(a, b);
}

u128_t operator -(const u128_t &a, const u128_t &b) {
  return safe_sub(a, b);
}

u128_t operator *(const u128_t &a, const u128_t &b) {
  u128_t ret;
  uint64_t h, l;
  if (a.val[1] && b.val[1])
    goto overflow;
  long_mul(a.val[0], b.val[0], ret.val[1], ret.val[0]);
  long_mul(a.val[1], b.val[0], h, l);
  if (h)
    goto overflow;
  ret.val[1]+=l;
  if (ret.val[1]<l)
    goto overflow;
  long_mul(a.val[0], b.val[1], h, l);
  if (h)
    goto overflow;
  ret.val[1]+=l;
  if (ret.val[1]<l)
    goto overflow;
  return ret;
  overflow:
  throw std::runtime_error{"Multiplication overflow"};
}

u128_t operator *(const u128_t &a, uint64_t b) {
  u128_t ret;
  if (a.val[1]) {
    uint64_t h, l;
    long_mul(a.val[0], b, ret.val[1], ret.val[0]);
    long_mul(a.val[1], b, h, l);
    if (h)
      goto overflow;
    ret.val[1]+=l;
    if (ret.val[1]<l)
      goto overflow;
  } else {
    long_mul(a.val[0], b, ret.val[1], ret.val[0]);
  }
  return ret;
  overflow:
  throw std::runtime_error{"Multiplication overflow"};
}

u128_t operator /(const u128_t &a, const u128_t &b) {
  u128_t q, r;
  div128by128(a, b, q, r);
  return q;
}

u128_t operator %(const u128_t &a, const u128_t &b) {
  u128_t q, r;
  div128by128(a, b, q, r);
  return r;
}

u128_t operator <<(const u128_t &a, uint32_t cnt) {
  cnt&=127;
  if (cnt) {
    if (cnt<64) {
      return u128_t(a.val[0]<<cnt, (a.val[1]<<cnt)|(a.val[0]>>(64-cnt)));
    } else if (cnt>64) {
      return u128_t(0, a.val[0]<<(cnt-64));
    } else {
      return u128_t(0, a.val[0]);
    }
  } else {
    return a;
  }
}

u128_t operator >>(const u128_t &a, uint32_t cnt) {
  cnt&=127;
  if (cnt) {
    if (cnt<64) {
      return u128_t((a.val[0]>>cnt)|(a.val[1]<<(64-cnt)), a.val[1]>>cnt);
    } else if (cnt>64) {
      return u128_t(a.val[1]>>(cnt-64), 0);
    } else {
      return u128_t(a.val[1], 0);
    }
  } else {
    return a;
  }
}

bool operator <(const u128_t &a, const u128_t &b) {
  return (a.val[1]==b.val[1])?(a.val[0]<b.val[0]):(a.val[1]<b.val[1]);
}

bool operator ==(const u128_t &a, const u128_t &b) {
  return a.val[0]==b.val[0] && a.val[1]==b.val[1];
}

bool operator <=(const u128_t &a, const u128_t &b) {
  // more optimal than a<b || a==b
  return (a.val[1]==b.val[1])?(a.val[0]<=b.val[0]):(a.val[1]<b.val[1]);
}

bool operator !=(const u128_t &a, const u128_t &b) {
  return !(a==b);
}

bool operator >(const u128_t &a, const u128_t &b) {
  return !(a<=b);
}

bool operator >=(const u128_t &a, const u128_t &b) {
  return !(a<b);
}

u128_t muldiv(const u128_t &a, const u128_t &b, uint64_t c, bool must_div) {
  uint64_t res[3], h, l, r;
  u128_t ret;
  res[2]=0;
  long_mul(a.val[0], b.val[0], res[1], res[0]);
  long_mul(a.val[1], b.val[0], res[2], l);
  res[1]+=l;
  if (res[1]<l)
    res[2]++;
  long_mul(a.val[0], b.val[1], h, l);
  res[1]+=l;
  if (res[1]<l && ++res[2]==0)
    goto overflow;
  res[2]+=h;
  if (res[2]<h)
    goto overflow;
  long_mul(a.val[1], b.val[1], h, l);
  res[2]+=l;
  if (h || res[2]<l || res[2]>=c)
    goto overflow;
  long_div64(res[2], res[1], c, ret.val[1], r);
  long_div64(r, res[0], c, ret.val[0], r);
  if (r && must_div)
    throw std::runtime_error{"Scaling factor does not evenly divide the product"};
  return ret;
  overflow:
  throw std::runtime_error{"Multiplication overflow"};

}

void div128by64(const u128_t &num, uint64_t den, u128_t &quotient, uint64_t &remainder) {
  uint64_t qh, l, h;
  if (num.val[1]>=den) {
    qh=num.val[1]/den;
    h=num.val[1]%den;
  } else {
    qh=0;
    h=num.val[1];
  }
  l=num.val[0];
  quotient.val[0]=long_div(h, l, den);
  quotient.val[1]=qh;
  remainder=h;
}

// based on https://github.com/hcs0/Hackers-Delight/blob/master/divDouble.c.txt
// does one long division (128bit/64bit) + either 64 bit division or two multiplications
void div128by128(const u128_t &num, const u128_t &den, u128_t &quotient, u128_t &remainder) {
  if (den.val[1]) {
    uint32_t n=__builtin_clzll(den.val[1]);
    u128_t v1=den<<n;
    u128_t u1=num>>1;
    uint64_t q=long_div(u1.val[1], u1.val[0], v1.val[1])>>(63-n);
    if (q)
      --q;
    remainder=unsafe_sub(num, unsafe_mul(den, q));
    if (remainder>=den) {
      ++q;
      remainder=unsafe_sub(remainder, den);
    }
    quotient=q;
  } else {
    remainder.val[1]=0;
    div128by64(num, den.val[0], quotient, remainder.val[0]);
  }
}
}
