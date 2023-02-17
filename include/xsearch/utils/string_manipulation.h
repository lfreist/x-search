// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <immintrin.h>

namespace xs::utils::simd {

/**
 * Convert src into lower case
 * @param src
 * @param size
 * @param dest
 */
void toLower(char* src, size_t size);

}  // namespace xs::utils::simd