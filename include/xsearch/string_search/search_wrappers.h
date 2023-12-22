/**
 * Copyright 2023, Leon Freist (https://github.com/lfreist)
 * Author: Leon Freist <freist.leon@gmail.com>
 *
 * This file is part of x-search.
 */

#pragma once

#include <re2/re2.h>
#include <xsearch/concepts.h>
#include <xsearch/string_search/simd_search.h>

#include <functional>
#include <iostream>
#include <vector>

namespace xs::search {

/**
 *
 * @tparam T
 * @param data
 * @param pattern
 * @param skip_to_nl
 * @param func
 * @return
 */
template <DefaultDataC T>
std::vector<uint64_t> _byte_offsets(
    const T& data, const std::string& pattern, bool skip_to_nl = true,
    const std::function<int64_t(uint64_t)>& func = [](uint64_t x) { return x; }) {
  std::vector<uint64_t> results;
  const char* pattern_c = pattern.data();
  size_t shift = 0;
  while (shift < data.size()) {
    int64_t match = simd::findNext(pattern_c, pattern.size(), data.data(), data.size(), shift);
    if (match == -1) {
      break;
    }
    results.push_back(func(match));
    shift = match + pattern.size();
    if (skip_to_nl) {
      match = simd::findNextNewLine(data.data(), data.size(), shift);
      if (match == -1) {
        break;
      }
      shift = match + 1;
    }
  }
  return results;
}

/**
 *
 * @tparam T
 * @param data
 * @param pattern
 * @param skip_to_nl
 * @param func
 * @return
 */
template <DefaultDataC T>
std::vector<uint64_t> _regex_byte_offsets(
    const T& data, const re2::RE2& pattern, bool skip_to_nl = true,
    const std::function<int64_t(uint64_t)>& func = [](uint64_t x) { return x; }) {
  std::vector<uint64_t> results;
  re2::StringPiece input(data.data(), data.size());
  re2::StringPiece match;
  size_t total_shift = 0;
  while (re2::RE2::PartialMatch(input, pattern, &match)) {
    size_t shift = match.data() - input.data();
    results.push_back(func(shift + total_shift));
    shift += match.size();
    input.remove_prefix(shift);
    if (skip_to_nl) {
      size_t next_nl = input.find('\n');
      if (next_nl == re2::StringPiece::npos) {
        break;
      }
      next_nl++;
      shift += next_nl;
      input.remove_prefix(next_nl);
    }
    total_shift += shift;
  }
  return results;
}

/**
 *
 * @param line
 * @param size
 * @param pattern
 * @return
 */
std::string _get_regex_match(const char* line, size_t size,
                             const re2::RE2& pattern) {
  re2::StringPiece input(line, size);
  re2::StringPiece match;
  re2::RE2::PartialMatch(input, pattern, &match);
  return match.as_string();
}

/**
 * Get the new line index of the previous line relative to the match
 * @param data
 * @param match_local_byte_offset
 * @return
 */
template <DefaultDataC T>
uint64_t previous_new_line_offset_relative_to_match(const T& data, uint64_t match_local_byte_offset) {
  uint64_t relative_offset = 0;
  while (true) {
    if (data.data()[match_local_byte_offset - relative_offset] == '\n') {
      return relative_offset - 1;
    }
    if (relative_offset >= match_local_byte_offset) {
      return match_local_byte_offset;
    }
    relative_offset++;
  }
}

/**
 * Search byte offsets (relative to start of data) of matches of pattern within
 * data. If a match was found, 'skip_to_nl' decides whether to continue search
 * in the next line or right behind the found pattern.
 *
 * @param data data to be searched on
 * @param pattern pattern to be searched for
 * @param skip_to_nl bool: true -> find at most one match per line, false ->
 * find all matches per line
 * @return std::vector<uint64_t>: vector of all found byte offsets
 */
template <DefaultDataC T>
std::vector<uint64_t> byte_offsets_match(const T& data, const std::string& pattern, bool skip_to_nl = false) {
  return _byte_offsets(data, pattern, skip_to_nl);
}

/**
 * Search byte offsets (relative to start of data) of lines containing a match
 * of pattern within data.
 *
 * @param data data to be searched on
 * @param pattern pattern to be searched for
 * @return std::vector<uint64_t>: vector of all found byte offsets
 */
template <DefaultDataC T>
std::vector<uint64_t> byte_offsets_line(const T& data, const std::string& pattern) {
  return _byte_offsets(data, pattern, true, [data](uint64_t v) {
    return v - previous_new_line_offset_relative_to_match(data, v);
  });
}

/**
 * Count the numbers of lines containing a match of pattern in data.
 *
 * @param data data to be searched in
 * @param pattern pattern to be searched for
 * @return number of matching lines within data
 */
template <DefaultDataC T>
uint64_t count(const T& data, const std::string& pattern, bool skip_to_nl = true) {
  uint64_t result = 0;
  const char* pattern_c = pattern.data();
  size_t shift = 0;
  while (shift < data.size()) {
    int64_t match = xs::search::simd::findNext(
        pattern_c, pattern.size(), data.data(), data.size(), shift);
    if (match == -1) {
      break;
    }
    result++;
    shift = match + pattern.size();
    if (skip_to_nl) {
      match = simd::findNextNewLine(data.data(), data.size(), shift);
      if (match == -1) {
        break;
      }
      shift = match + 1;
    }
  }
  return result;
}

template <DefaultDataC T>
std::vector<std::string> line(const T& data, const std::string& pattern) {
  std::vector<std::string> results;
  const char* pattern_c = pattern.data();
  size_t shift = 0;
  while (shift < data.size()) {
    int64_t match = simd::findNext(pattern_c, pattern.size(), data.data(), data.size(), shift);
    if (match == -1) {
      break;
    }
    int64_t line_begin = match - previous_new_line_offset_relative_to_match(data, match);
    shift = match + pattern.size();
    int64_t line_end = simd::findNextNewLine(data.data(), data.size(), shift);
    if (line_end == -1) {
      break;
    }
    shift = line_end + 1;
    results.push_back(std::string(data.data() + line_begin, line_end-line_begin));
  }
  return results;
}

namespace regex {

/**
 * Search byte offsets (relative to start of data) of lines containing a regex
 * match of pattern within data.
 *
 * @param data data to be searched on
 * @param pattern const re2::RE2&: pattern to be searched for
 * @return std::vector<uint64_t>: vector of all found byte offsets
 */
template <DefaultDataC T>
std::vector<uint64_t> byte_offsets_line(const T& data, const re2::RE2& pattern) {
  return _regex_byte_offsets(data, pattern, true, [data](uint64_t v) {
    return v - previous_new_line_offset_relative_to_match(data, v);
  });
}

/**
 * Search byte offsets (relative to start of data) of regex matches of pattern
 * within data. If a match was found, 'skip_to_nl' decides whether to continue
 * search in the next line or right behind the found pattern.
 *
 * @param data data to be searched on
 * @param pattern const re2::RE2&: pattern to be searched for
 * @param skip_to_nl bool: true -> find at most one match per line, false ->
 * find all matches per line
 * @return std::vector<uint64_t>: vector of all found byte offsets
 */
template <DefaultDataC T>
std::vector<uint64_t> byte_offsets_match(const T& data, const re2::RE2& pattern, bool skip_to_nl = false) {
  return _regex_byte_offsets(data, pattern, skip_to_nl);
}

/**
 * Count the numbers of lines containing a regex match of pattern in data.
 *
 * @param data data to be searched in
 * @param pattern const re2::RE2&: pattern to be searched for
 * @return number of matching lines within data
 */
template <DefaultDataC T>
uint64_t count(const T& data, const re2::RE2& pattern, bool skip_to_nl = true) {
  uint64_t counter = 0;
  re2::StringPiece input(data.data(), data.size());
  re2::StringPiece match;
  while (re2::RE2::PartialMatch(input, pattern, &match)) {
    size_t shift = match.data() - input.data();
    counter++;
    shift += match.size();
    input.remove_prefix(shift);
    if (skip_to_nl) {
      size_t next_nl = input.find('\n');
      if (next_nl == re2::StringPiece::npos) {
        break;
      }
      next_nl++;
      input.remove_prefix(next_nl);
    }
  }
  return counter;
}

}  // namespace regex

}  // namespace xs::search