// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/utils/string_utils.h>

#include <cctype>
#include <cstring>

namespace xs::utils::str::simd {

void toLower(char* src, size_t size) {
  __m256i diff = _mm256_set1_epi8('a' - 'A');
  __m256i A = _mm256_set1_epi8('A' - 1);
  __m256i Z = _mm256_set1_epi8('Z' + 1);
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
    *src = static_cast<char>(std::tolower(*src));
    size--;
    src++;
  }
}

}  // namespace xs::utils::str::simd

namespace xs::utils::str {

bool is_ascii(const std::string& str) {
  return std::all_of(str.begin(), str.end(),
                     [](char c) { return static_cast<u_char>(c) <= 127; });
}

bool is_utf8(const std::string& str) {
  int num_bytes = 0;
  for (const char c : str) {
    if (num_bytes == 0) {
      if ((c & 0x80) == 0) {
        // is ASCII char -> valid
        continue;
      } else if ((c & 0xE0) == 0xC0) {
        // two byte char
        num_bytes = 1;
      } else if ((c & 0xf0) == 0xE0) {
        // three byte char
        num_bytes = 2;
      } else if ((c & 0xF8) == 0xF0) {
        // four byte char
        num_bytes = 3;
      } else {
        // invalid start byte
        return false;
      }
    } else {
      if ((c & 0xC0) != 0x80) {
        // invalid continuation byte for UTF-8
        return false;
      }
      num_bytes--;
    }
  }
  return num_bytes == 0;
}

bool is_utf8(const char* data, size_t size) {
  int num_bytes = 0;
  for (size_t i = 0; i < size; ++i) {
    if (num_bytes == 0) {
      if ((data[i] & 0x80) == 0) {
        // is ASCII char -> valid
        continue;
      } else if ((data[i] & 0xE0) == 0xC0) {
        // two byte char
        num_bytes = 1;
      } else if ((data[i] & 0xf0) == 0xE0) {
        // three byte char
        num_bytes = 2;
      } else if ((data[i] & 0xF8) == 0xF0) {
        // four byte char
        num_bytes = 3;
      } else {
        // invalid start byte
        return false;
      }
    } else {
      if ((data[i] & 0xC0) != 0x80) {
        // invalid continuation byte for UTF-8
        return false;
      }
      num_bytes--;
    }
  }
  return num_bytes == 0;
}

std::string escaped(const std::string& str) {
  std::string escaped;
  for (char c : str) {
    if (c == '\\' || c == '.' || c == '[' || c == ']' || c == '(' || c == ')' ||
        c == '{' || c == '}' || c == '|' || c == '*' || c == '+' || c == '?' ||
        c == '^' || c == '$') {
      escaped.push_back('\\');
    }
    escaped.push_back(c);
  }
  return escaped;
}

}  // namespace xs::utils::str