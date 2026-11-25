#ifndef FIXED_PT_H
#define FIXED_PT_H

#include <stdint.h>

typedef int64_t fix64;

#define FRAC_WIDTH 32 // non configurable for mult operation

#define GET_INT(fix) ((fix) >> FRAC_WIDTH)
#define GET_FRAC(fix) ((fix) & ((1LL << FRAC_WIDTH) - 1))

#define FIX_ONE (1LL << FRAC_WIDTH)
#define FIX_ONE_HALF (1LL << (FRAC_WIDTH - 1))
#define FIX_MAX (fix64)((1ULL << 63) - 1)                       // full fraction, max value integer
#define FIX_MIN (fix64)(1ULL << 63) // full fraction, min value integer

#define FIX_2_INT(fix) (((fix) >> FRAC_WIDTH)) // non rounding
#define FIX_2_FLOAT(fix) ((float) (fix) / (1LL << FRAC_WIDTH))
#define FIX_2_DOUBLE(fix) ((double) (fix) / (1LL << FRAC_WIDTH))

#define INT_2_FIX(num) ((fix64)(num) << FRAC_WIDTH) // all fixed point operations must involve two fix64 numbers
#define FLOAT_2_FIX(f) ((fix64)((f) * (1LL << FRAC_WIDTH))) // shifting float left FRAC_WIDTH bits
#define DOUBLE_2_FIX(d) ((fix64) ((d) * (1LL << FRAC_WIDTH))) // same as above

#define FIX_INV(fix) ((1ULL << 63) / (fix) << 1) // only loses one bit of precision (?)

static inline fix64 fix_mult(const fix64 fix1, const fix64 fix2) { // multiply 2 fixes, return fix
    fix64 i1 = GET_INT(fix1);
    fix64 f1 = GET_FRAC(fix1);
    fix64 i2 = GET_INT(fix2);
    fix64 f2 = GET_FRAC(fix2);
    
    return (((uint64_t) f1 * f2) >> 32) 
        + ((int64_t)(i1 * i2) << 32)
        + i1 * f2
        + i2 * f1;
}

static inline int32_t fix_mult_i64(const fix64 fix1, const fix64 fix2) { // multiply two fixes, return integer part. saves time not calculating decimal * decimal
    fix64 i1 = GET_INT(fix1);
    fix64 f1 = GET_FRAC(fix1);
    fix64 i2 = GET_INT(fix2);
    fix64 f2 = GET_FRAC(fix2);
    
    return (int32_t)i1 * (int32_t)i2 
        + (int32_t)((i1 * f2 + i2 * f1) >> 32);
}

static inline int32_t fix_mult_i32(const fix64 fix1, const fix64 fix2) { // save more time by "casting" to 16.16 fixed point. Loss of precision, obviously
    return ((fix1 >> 16) * (fix2 >> 16)) >> 32;
}

// assumes non zero denominators
static inline fix64 fix_div_s(const fix64 num, const fix64 denom) { // much easier than finding fast integer division (?)
    return (fix64)((float)num / FIX_2_FLOAT(denom)); // single precision
}

static inline fix64 fix_div_d(const fix64 num, const fix64 denom) {
    return (fix64)((double)num / FIX_2_DOUBLE(denom)); // double precision
}

/* Integer reciprocal and division.
 * The CPU has no FPU, so fix_div_s() costs several soft-float library calls (int64 -> float,
 * fdiv, float -> int64), and FIX_INV() a 64-bit integer division. These do the same job with
 * a small table, two Newton-Raphson steps and a few 32x32->64 multiplies. Relative error is
 * below 2^-26 (single-precision float is 2^-24), and exact powers of two give exact results,
 * so FIX_ONE stays FIX_ONE. */

// round(2^16 / (1 + (i + 0.5) / 64)): 1/M at the middle of each of 64 slices of M in [1, 2)
static const uint16_t fix_recip_tab[64] = {
    65028, 64035, 63072, 62138, 61231, 60350, 59494, 58662,
    57852, 57065, 56299, 55554, 54828, 54120, 53431, 52759,
    52103, 51464, 50840, 50231, 49637, 49056, 48489, 47935,
    47393, 46864, 46346, 45839, 45344, 44859, 44384, 43919,
    43464, 43019, 42582, 42154, 41734, 41323, 40920, 40525,
    40137, 39756, 39383, 39017, 38657, 38304, 37958, 37617,
    37283, 36954, 36631, 36314, 36003, 35696, 35395, 35099,
    34808, 34521, 34239, 33962, 33689, 33421, 33157, 32897,
};

// m in [2^31, 2^32), i.e. M = m / 2^31 in [1, 2). Returns 1/M in Q1.31, about 2^62 / m,
// in (2^30, 2^31]. Relative error < 2^-27 either side. The 2^31 upper bound was checked over
// every input; fix_div() relies on it.
static inline uint32_t fix_recip32(const uint32_t m) {
    if (m == 0x80000000u) {
        return 0x80000000u; // M == 1 exactly
    }
    uint32_t r = (uint32_t) fix_recip_tab[(m >> 25) & 63] << 15; // ~7 correct bits
    for (int i = 0; i < 2; i++) {                                 // ~14, then ~28 bits
        const uint32_t mr = (uint32_t) (((uint64_t) m * r) >> 31); // M*R, close to 1.0 in Q1.31
        r = (uint32_t) (((uint64_t) r * (0u - mr)) >> 31);         // R * (2 - M*R)
    }
    return r;
}

// 1 / x for x > 0. Returns FIX_MAX when x <= 0 or the result doesn't fit.
static inline fix64 fix_recip(const fix64 x) {
    if (x <= 0) {
        return FIX_MAX;
    }
    const int n = __builtin_clzll((uint64_t) x); // 1..63
    if (n >= 62) {
        return FIX_MAX; // x < 2^-30
    }
    const uint32_t m = (uint32_t) (((uint64_t) x << n) >> 32);
    const uint64_t r = fix_recip32(m);
    // raw result is 2^64 / x = 2^(32 + n) / m = r * 2^(n - 30)
    return n >= 30 ? (fix64) (r << (n - 30)) : (fix64) (r >> (30 - n));
}

// num / denom, relative error < 2^-26 either way (so x / x can give FIX_ONE + 1).
// Division by zero or an out-of-range result saturates
// to FIX_MAX or FIX_MIN by sign (0 / 0 gives 0).
static inline fix64 fix_div(const fix64 num, const fix64 denom) {
    const int neg = (num ^ denom) < 0;
    const uint64_t un = num < 0 ? 0 - (uint64_t) num : (uint64_t) num;
    const uint64_t ud = denom < 0 ? 0 - (uint64_t) denom : (uint64_t) denom;
    if (un == 0) {
        return 0;
    }
    if (ud == 0) {
        return neg ? FIX_MIN : FIX_MAX;
    }
    const int nn = __builtin_clzll(un);
    const int nd = __builtin_clzll(ud);
    const uint32_t mn = (uint32_t) ((un << nn) >> 32);
    const uint32_t md = (uint32_t) ((ud << nd) >> 32);
    uint64_t q = (uint64_t) mn * fix_recip32(md); // (mn / md) * 2^62, in (2^61, 2^63)
    const int s = nd - nn - 30;                   // raw quotient = q * 2^s
    if (s > 0) {
        if ((q >> (63 - s)) != 0) {
            return neg ? FIX_MIN : FIX_MAX;
        }
        q <<= s;
    } else if (s < 0) {
        q = s <= -64 ? 0 : q >> -s;
    }
    return neg ? -(fix64) q : (fix64) q;
}


#endif