// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <regex>
#include <string>

namespace xs::utils {

/**
 * Check whether a pattern should be used as regex or as plain text.
 * pattern is used as regex if it contains regex specific characters.
 * @param pattern
 * @return bool: true if regex, else false
 */
inline bool use_str_as_regex(const std::string& pattern) {
  try {
    std::regex r(pattern);
    return !(std::regex_match(pattern, std::regex("^" + pattern + "$")));
  } catch (std::regex_error& e) {
    return false;
  }
}

}  // namespace xs::utils