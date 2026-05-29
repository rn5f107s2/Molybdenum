#pragma once

#include <immintrin.h>
#include <cstdint>

#ifdef __AVX2__

using vec_t     = __m256i;
using halfvec_t = __m128i;

#define vec_loadu  _mm256_loadu_si256

#define vec_storeu _mm256_storeu_si256

#define vec_add_epi32 _mm256_add_epi32
#define vec_add_epi16 _mm256_add_epi16
#define vec_sub_epi16 _mm256_sub_epi16

#define vec_madd_epi16 _mm256_madd_epi16
#define vec_mullo_epi16 _mm256_mullo_epi16

#define vec_max_epi16 _mm256_max_epi16
#define vec_min_epi16 _mm256_min_epi16

#define vec_setzero _mm256_setzero_ps
#define vet_set1_epi16 _mm256_set1_epi16

inline vec_t vec_loadu2(const halfvec_t* hi, const halfvec_t* lo) {
    return _mm256_loadu2_m128i(hi, lo);
}

// https://stackoverflow.com/a/35270026
inline int hsum(__m256i v) {
    __m128i hi128 = _mm256_extracti128_si256(v, 1);
    __m128i lo128 = _mm256_castsi256_si128(v);
    __m128i sum128 = _mm_add_epi32(hi128, lo128);
    __m128i sum64 = _mm_add_epi32(sum128, _mm_unpackhi_epi64(sum128, sum128));
    __m128i sum32 = _mm_add_epi32(sum64, _mm_shufflelo_epi16(sum64, _MM_SHUFFLE(1, 0, 3, 2)));

    return _mm_cvtsi128_si32(sum32);
}

inline void dpbusd(vec_t& sum, vec_t inputs, vec_t weights) {
    vec_t prod = _mm256_maddubs_epi16(inputs, weights);
    vec_t wide = _mm256_madd_epi16(prod, _mm256_set1_epi16(1));

    sum = _mm256_add_epi32(sum, wide);
}

#else 

using vec_t     = __m128i;
using halfvec_t = int64_t;

#define vec_loadu _mm_loadu_si128

#define vec_storeu _mm_storeu_si128

#define vec_add_epi32 _mm_add_epi32
#define vec_add_epi16 _mm_add_epi16
#define vec_sub_epi16 _mm_sub_epi16

#define vec_madd_epi16 _mm_madd_epi16
#define vec_mullo_epi16 _mm_mullo_epi16

#define vec_max_epi16 _mm_max_epi16
#define vec_min_epi16 _mm_min_epi16

#define vec_setzero _mm_setzero_ps
#define vet_set1_epi16 _mm_set1_epi16

inline vec_t vec_loadu2(const halfvec_t* hi, const halfvec_t* lo) {
    vec_t vec_lo = _mm_loadl_epi64((vec_t*) lo);
    vec_t vec_hi = _mm_loadl_epi64((vec_t*) hi);

    return _mm_unpacklo_epi64(vec_lo, vec_hi);
}

// https://stackoverflow.com/a  /35270026
inline int hsum(vec_t v) {
    __m128i sum64 = _mm_add_epi32(v, _mm_unpackhi_epi64(v, v));
    __m128i sum32 = _mm_add_epi32(sum64, _mm_shufflelo_epi16(sum64, _MM_SHUFFLE(1, 0, 3, 2)));

    return _mm_cvtsi128_si32(sum32);
}

inline void dpbusd(vec_t& sum, vec_t inputs, vec_t weights) {
    (void) inputs; (void) weights; (void) sum;
}

#endif