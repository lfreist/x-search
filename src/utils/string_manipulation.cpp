// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/utils/string_manipulation.h>

#include <cctype>
#include <cstring>

namespace xs::utils::simd {

void toLower(char* src, size_t size) {
  __m256i diff = _mm256_set1_epi8('a' - 'A');
  __m256i A = _mm256_set1_epi8('A');
  __m256i Z = _mm256_set1_epi8('Z');
  while (size >= 32) {
    const __m256i data =
        _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src));
    const __m256i greater_than_A = _mm256_cmpgt_epi8(data, A);
    const __m256i less_equal_Z = _mm256_cmpgt_epi8(Z, data);
    const __m256i mask = _mm256_and_si256(greater_than_A, less_equal_Z);
    const __m256i to_be_added = _mm256_and_si256(mask, diff);
    const __m256i result = _mm256_add_epi8(data, to_be_added);
    _mm256_storeu_si256((__m256i*)src, result);
    size -= 32;
    src += 32;
  }
  // convert remaining data without using SIMD
  while (size > 0) {
    *src = std::tolower(*src);
    size--;
    src++;
  }
}

}  // namespace xs::utils::simd