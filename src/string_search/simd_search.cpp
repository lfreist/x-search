// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <immintrin.h>
#include <xsearch/string_search/simd_search.h>

#include <cstring>
#include <iostream>

namespace xs::search::simd {

char* strchr(char* str, size_t str_len, char c) {
  // we are using 256 bits (32 bytes) vectors. If str_len is smaller than 32, we
  // just perform std::strchr
  if (str_len < 32) {
    return std::strchr(str, c);
  }
  // load c into SIMD vector
  const __m256i _c = _mm256_set1_epi8(c);

  // we are using 256 bits (32 bytes) vectors. If remaining str is smaller than
  // 32, we stop and perform std::strchr on the remaining str
  while (str_len >= 32) {
    // load next 32 bytes
    const __m256i block =
        _mm256_loadu_si256(reinterpret_cast<const __m256i*>(str));

    const __m256i eq = _mm256_cmpeq_epi8(_c, block);

    // create mask
    uint32_t mask = _mm256_movemask_epi8(eq);
    if (mask != 0) {
      const unsigned bitpos =
          __builtin_ctz(mask);  // get index of first true value
      return str + bitpos;      // return position of match
    }
    str_len -= 32;
    str += 32;
  }
  // perform std::strchr on remaining str
  return std::strchr(str, c);
}

char* strstr(char* str, size_t str_len, const char* pattern,
             size_t pattern_len) {
  // we are using 256 bits (32 bytes) vectors. If str_len is smaller than 32, we
  // just perform std::strstr
  if (str_len < 32) {
    return std::strstr(str, pattern);
  }
  // load first char of pattern
  const __m256i first = _mm256_set1_epi8(pattern[0]);
  // load last char of pattern
  const __m256i last = _mm256_set1_epi8(pattern[pattern_len - 1]);

  // we are using 256 bits (32 bytes) vectors. If remaining str is smaller than
  // 32, we stop and perform std::strstr on the remaining str
  while (str_len >= 32) {
    // load next 32 bytes
    const __m256i block_first =
        _mm256_loadu_si256(reinterpret_cast<const __m256i*>(str));
    // load 32 bytes stating at offset (pattern_len -1)
    const __m256i block_last = _mm256_loadu_si256(
        reinterpret_cast<const __m256i*>(str + pattern_len - 1));

    // create bit masks
    const __m256i eq_first = _mm256_cmpeq_epi8(first, block_first);
    const __m256i eq_last = _mm256_cmpeq_epi8(last, block_last);

    // logical and bit masks
    uint32_t mask = _mm256_movemask_epi8(_mm256_and_si256(eq_first, eq_last));

    while (mask != 0) {
      const unsigned bitpos =
          __builtin_ctz(mask);  // get index of first true value
      // compare all bytes if a match was found
      if (memcmp(str + bitpos + 1, pattern + 1, pattern_len - 1) == 0) {
        // return position of found match
        return str + bitpos;
      }
      mask = mask & (mask - 1);  // remove all false values from vector
    }
    // decrease str_len by number of read bytes
    str_len -= 32;
    // shift str pointer to new start
    str += 32;
  }
  return std::strstr(str, pattern);
}

int64_t findNext(const char* pattern, size_t pattern_len, char* str,
                 size_t str_len, size_t shift) {
  char* match = strstr(str + shift, str_len - shift, pattern, pattern_len);
  return match == nullptr ? -1 : match - str;
}

int64_t findNextNewLine(char* str, size_t str_len, size_t shift) {
  char* match = strchr(str + shift, str_len - shift, '\n');
  return match == nullptr ? -1 : match - str;
}

uint64_t findAllPerLine(const char* pattern, size_t pattern_len, char* str,
                        size_t str_len) {
  uint64_t count = 0;
  size_t shift = 0;
  while (true) {
    int64_t match = findNext(pattern, pattern_len, str, str_len, shift);
    if (match == -1) {
      break;
    }
    count++;
    shift = match + pattern_len;
    match = findNextNewLine(str, str_len, shift);
    if (match == -1) {
      break;
    }
    shift = ++match;
  }
  return count;
}

uint64_t findAll(const char* pattern, size_t pattern_len, char* str,
                 size_t str_len) {
  uint64_t count = 0;
  size_t shift = 0;
  while (true) {
    int64_t match = findNext(pattern, pattern_len, str, str_len, shift);
    if (match == -1) {
      break;
    }
    count++;
    shift = match + pattern_len;
  }
  return count;
}

}  // namespace xs::search::simd
