// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <immintrin.h>
#endif  // _MSC_VER

#include <xsearch/string_search/simd_search.h>

#include <cstring>
#include <iostream>

namespace xs::search::simd {

inline int count_trailing_zeroes(unsigned int n) {
#ifdef _MSC_VER
  unsigned bits = 0;
  unsigned x = n;

  if (x) {
    /* assuming `x` has 32 bits: lets count the low order 0 bits in batches */
    /* mask the 16 low order bits, add 16 and shift them out if they are all 0 */
    if (!(x & 0x0000FFFF)) {
      bits += 16;
      x >>= 16;
    }
    /* mask the 8 low order bits, add 8 and shift them out if they are all 0 */
    if (!(x & 0x000000FF)) {
      bits += 8;
      x >>= 8;
    }
    /* mask the 4 low order bits, add 4 and shift them out if they are all 0 */
    if (!(x & 0x0000000F)) {
      bits += 4;
      x >>= 4;
    }
    /* mask the 2 low order bits, add 2 and shift them out if they are all 0 */
    if (!(x & 0x00000003)) {
      bits += 2;
      x >>= 2;
    }
    /* mask the low order bit and add 1 if it is 0 */
    bits += (x & 1) ^ 1;
  }
  return static_cast<int>(bits);
#else
  return __builtin_ctz(n);
#endif
}

/// simple strstr implementation for char* that is not null terminated
const char* rest_strstr(const char* str, size_t str_len, const char* pattern, size_t pat_len) {
  size_t shift = 0;
  while (shift < str_len) {
    if (str_len - shift < pat_len) {
      return nullptr;
    }  // not found
    bool flag = true;
    size_t str_index = shift;
    for (size_t p_i = 0; p_i < pat_len; ++p_i) {
      if (str[str_index++] != pattern[p_i]) {
        flag = false;
        break;
      }
    }
    if (flag) {
      return str + shift;  // match found
    }
    shift = str_index;
  }
  return nullptr;
}

const char* rest_strcasestr(const char* str, size_t str_len, const char* pattern, size_t pat_len) {
  size_t shift = 0;
  while (shift < str_len) {
    if (str_len - shift < pat_len) {
      return nullptr;
    }  // not found
    bool flag = true;
    size_t str_index = shift;
    for (size_t p_i = 0; p_i < pat_len; ++p_i) {
      // this is not optimized but since this function is only called if str_len
      //  < 32, it doesn't really matter
      char lower_str = static_cast<char>(std::tolower(str[str_index++]));
      char lower_pat = static_cast<char>(std::tolower(pattern[p_i]));
      if (lower_str != lower_pat) {
        flag = false;
        break;
      }
    }
    if (flag) {
      return str + shift;  // match found
    }
    shift = str_index;
  }
  return nullptr;
}

/// simple implementation of strchr for char* that is not null terminated
const char* rest_strchr(const char* str, size_t str_len, int c) {
  for (size_t i = 0; i < str_len; ++i) {
    if (str[i] == c) {
      return str + i;
    }
  }
  return nullptr;
}

const char* strchr(const char* str, size_t str_len, char c) {
  // we are using 256 bits (32 bytes) vectors. If str_len is smaller than 32, we
  // just perform std::strchr
  if (str_len < 32) {
    return rest_strchr(str, str_len, c);
  }
  // load c into SIMD vector
  const __m256i _c = _mm256_set1_epi8(c);

  // we are using 256 bits (32 bytes) vectors. If remaining str is smaller than
  // 32, we stop and perform std::strchr on the remaining str
  while (str_len >= 32) {
    // load next 32 bytes
    const __m256i block = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(str));

    const __m256i eq = _mm256_cmpeq_epi8(_c, block);

    // create mask
    uint32_t mask = _mm256_movemask_epi8(eq);
    if (mask != 0) {
      const unsigned bitpos = count_trailing_zeroes(mask);  // get index of first true value
      return str + bitpos;                                  // return position of match
    }
    str_len -= 32;
    str += 32;
  }
  // perform rest_strchr on remaining str
  return rest_strchr(str, str_len, c);
}

/// helper function for strstr
uint32_t get_mask(const __m256i first, const __m256i last, const __m256i block_first, const __m256i block_last) {
  const __m256i eq_first = _mm256_cmpeq_epi8(first, block_first);
  const __m256i eq_last = _mm256_cmpeq_epi8(last, block_last);
  return _mm256_movemask_epi8(_mm256_and_si256(eq_first, eq_last));
}

bool compare_case_insensitive(const char* str, const char* pattern, size_t len) {
  for (size_t i = 0; i < len; ++i) {
    if (std::tolower(str[i]) != std::tolower(pattern[i])) {
      return false;
    }
  }
  return true;
}

const char* strstr(const char* str, size_t str_len, const char* pattern, size_t pattern_len) {
  // use strchr if pattern is of size 1
  if (pattern_len == 1) {
    return strchr(str, str_len, pattern[0]);
  }
  // we are using 256 bits (32 bytes) vectors. If str_len is smaller than 32, we
  // just perform std::strstr
  if (str_len < 32 + pattern_len) {
    return rest_strstr(str, str_len, pattern, pattern_len);
  }
  // load first char of pattern
  const __m256i first = _mm256_set1_epi8(pattern[0]);
  // load last char of pattern
  const __m256i last = _mm256_set1_epi8(pattern[pattern_len - 1]);

  // we are using 256 bits (32 bytes) vectors. If remaining str is smaller than
  // 32, we stop and perform std::strstr on the remaining str
  while (str_len >= 32 + pattern_len) {
    // load next 32 bytes
    const __m256i block_first = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(str));
    // load 32 bytes stating at offset (pattern_len -1)
    const __m256i block_last = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(str + pattern_len - 1));

    // create bit masks
    uint32_t mask = get_mask(first, last, block_first, block_last);

    while (mask != 0) {
      const unsigned bitpos = count_trailing_zeroes(mask);  // get index of first true value
      // compare all bytes if a match was found
      // TODO: this could be pattern_len -2 right?
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
  return rest_strstr(str, str_len, pattern, pattern_len);
}

const char* make_compare_icase(uint32_t mask, const char* str, const char* pattern, size_t pattern_len) {
  while (mask != 0) {
    // get index of first true value
    const unsigned bitpos = count_trailing_zeroes(mask);
    // compare all bytes if a match was found
    if (compare_case_insensitive(str + bitpos + 1, pattern + 1, pattern_len - 2)) {
      // return position of found match
      return str + bitpos;
    }
    mask = mask & (mask - 1);  // remove all false values from vector
  }
  return nullptr;
}

const char* strcasestr(const char* str, size_t str_len, const char* pat, size_t pat_len) {
  // we are using 256 bits (32 bytes) vectors. If str_len is smaller than 32, we
  // just perform std::strstr
  if (str_len < 32 + pat_len) {
    return rest_strcasestr(str, str_len, pat, pat_len);
  }
  // load first char of pattern in lower and upper case
  const __m256i first_lower = _mm256_set1_epi8(static_cast<char>(std::tolower(pat[0])));
  const __m256i first_upper = _mm256_set1_epi8(static_cast<char>(std::toupper(pat[0])));
  // load last char of pattern in lower and upper case
  const __m256i last_lower = _mm256_set1_epi8(static_cast<char>(std::tolower(pat[pat_len - 1])));
  const __m256i last_upper = _mm256_set1_epi8(static_cast<char>(std::toupper(pat[pat_len - 1])));

  // we are using 256 bits (32 bytes) vectors. If remaining str is smaller than
  // 32, we stop and perform std::strstr on the remaining str

  const char** results = new const char*[4];

  while (str_len >= 32 + pat_len) {
    // load next 32 bytes
    const __m256i block_first = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(str));
    // load 32 bytes stating at offset (pattern_len -1)
    const __m256i block_last = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(str + pat_len - 1));

    // create mask and compare for all 4 possible case combinations
    // get result fo a...a
    uint32_t mask = get_mask(first_lower, last_lower, block_first, block_last);
    const char* res = make_compare_icase(mask, str, pat, pat_len);
    results[0] = res;
    // A...a
    mask = get_mask(first_upper, last_lower, block_first, block_last);
    res = make_compare_icase(mask, str, pat, pat_len);
    results[1] = res;
    // A...A
    mask = get_mask(first_upper, last_upper, block_first, block_last);
    res = make_compare_icase(mask, str, pat, pat_len);
    results[2] = res;
    // a...A
    mask = get_mask(first_lower, last_upper, block_first, block_last);
    res = make_compare_icase(mask, str, pat, pat_len);
    results[3] = res;

    const char* first = nullptr;
    for (int i = 0; i < 4; ++i) {
      if (results[i] == nullptr) {
        continue;
      }
      if (first == nullptr) {
        first = results[i];
        continue;
      }
      if (first > results[i]) {
        first = results[i];
      }
    }
    if (first != nullptr) {
      delete[] results;
      return first;
    }

    // decrease str_len by number of read bytes
    str_len -= 32;
    // shift str pointer to new start
    str += 32;
  }
  delete[] results;
  return rest_strcasestr(str, str_len, pat, pat_len);
}

int64_t findNext(const char* pattern, size_t pattern_len, char* str, size_t str_len, size_t shift) {
  const char* match = strstr(str + shift, str_len - shift, pattern, pattern_len);
  return match == nullptr ? -1 : match - str;
}

int64_t findNextNewLine(const char* str, size_t str_len, size_t shift) {
  const char* match = strchr(str + shift, str_len - shift, '\n');
  return match == nullptr ? -1 : match - str;
}

uint64_t findAllPerLine(const char* pattern, size_t pattern_len, char* str, size_t str_len) {
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

uint64_t findAll(const char* pattern, size_t pattern_len, char* str, size_t str_len) {
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
