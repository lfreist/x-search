// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <string>

namespace xs::search::simd {

/**
 * std::strchr implementation using simd instruction set (AVX). Additionally to
 * the std::strchr specification, the size of str must be provided to avoid
 * inhibited memory access (segmentation fault, when avoiding running
 * std::strlen in simd::strchr)
 *
 * @param str data string
 * @param str_len size of str
 * @param c int to be searched for in str
 * @return pointer to match
 */
const char* strchr(const char* str, size_t str_len, char c);

/**
 * std::strstr implementation using simd instruction set (AVX). Additionally to
 * the std::strchr specification, the size of str and pattern must be provided
 * to avoid inhibited memory access (segmentation fault, when avoiding running
 * std::strlen in simd::strstr)
 *
 * @param str data string
 * @param str_len size of str
 * @param pattern pattern to be searched for in str
 * @param pattern_len size of pattern
 * @return pointer to match
 */
const char* strstr(const char* str, size_t str_len, const char* pattern,
                   size_t pattern_len);

const char* strcasestr(const char* str, size_t str_len, const char* pat,
                       size_t pat_len);

/**
 * simd::strstr wrapper for getting offset of the next match of pattern in
 * respect to the start of str.
 *
 * @param pattern pattern to be searched for in str
 * @param pattern_len size of pattern
 * @param str data string
 * @param str_len size of str
 * @param shift indicates where to start the search in str (pointer addition
 * performed)
 * @return offset of match with respect to start of str
 */
int64_t findNext(const char* pattern, size_t pattern_len, char* str,
                 size_t str_len, size_t shift);

/**
 * simd::strchr wrapper for getting the offset of the next new line char (\n)
 * with respect to the start of str.
 *
 * @param str data string
 * @param str_len size of str
 * @param shift indicates, where to start the search in str (pointer addition
 * performed)
 * @return offset of match with respect to start of str
 */
int64_t findNextNewLine(const char* str, size_t str_len, size_t shift);

/**
 * Counts the occurrences of pattern in str, moving to next new line char (\n)
 * whenever a match was found -> Count the occurrences of pattern in str per
 * line.
 *
 * @param pattern pattern to be searched for in str
 * @param pattern_len size of pattern
 * @param str data string
 * @param str_len size of data
 * @return number of occurrences of pattern in str per line
 */
uint64_t findAllPerLine(const char* pattern, size_t pattern_len, char* str,
                        size_t str_len);

/**
 * Counts the number of occurrences of pattern in str.
 *
 * @param pattern pattern to be searched for in str
 * @param pattern_len size of pattern
 * @param str data string
 * @param str_len size of data
 * @return number of occurrences of pattern in str
 */
uint64_t findAll(const char* pattern, size_t pattern_len, char* str,
                 size_t str_len);

}  // namespace xs::search::simd