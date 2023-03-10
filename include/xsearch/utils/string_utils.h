// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <immintrin.h>

#include <algorithm>
#include <string>

namespace xs::utils::str {

namespace simd {
/**
 * Convert src into lower case
 * @param src
 * @param size
 * @param dest
 */
void toLower(char* src, size_t size);

}  // namespace simd

bool is_ascii(const std::string& str);

bool is_utf8(const std::string& str);

bool is_utf8(const char* data, size_t size);

std::string escaped(const std::string& str);

}  // namespace xs::utils::str