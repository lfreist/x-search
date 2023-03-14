// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <nanobench.h>
#include <re2/re2.h>
#include <xsearch/string_search/simd_search.h>
#include <xsearch/utils/string_utils.h>

#include <cassert>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>

#define SIZE 16777216

// Functions that are benchmarked ----------------------------------------------

/// Google RE2
uint64_t re2_find(const std::string& content, const std::string& pattern) {
  uint64_t count = 0;
  re2::RE2::Options options;
  options.set_case_sensitive(false);
  re2::RE2 re_pattern('(' + pattern + ')', options);
  re2::StringPiece input(content.data(), content.size());
  re2::StringPiece match;
  while (re2::RE2::FindAndConsume(&input, re_pattern, &match)) {
    ++count;
  }
  return count;
}

/// strcasestr
uint64_t c_strcasestr(const std::string& content, const std::string& pattern) {
  int64_t result = 0;
  size_t shift = 0;
  while (true) {
    const char* match = strcasestr(content.c_str() + shift, pattern.c_str());
    if (match == nullptr) {
      break;
    }
    result++;
    shift = (match - content.data()) + pattern.size();
  }
  return result;
}

/// std::tolower + std::strstr
uint64_t std_lower_strstr(const std::string& content, std::string pattern) {
  int64_t result = 0;
  size_t shift = 0;
  std::string lower_content;
  lower_content.reserve(content.size());
  std::transform(content.begin(), content.end(), lower_content.begin(),
                 ::tolower);
  std::transform(pattern.begin(), pattern.end(), pattern.begin(), ::tolower);
  while (true) {
    const char* match =
        strcasestr(lower_content.c_str() + shift, pattern.c_str());
    if (match == nullptr) {
      break;
    }
    result++;
    shift = (match - lower_content.data()) + pattern.size();
  }
  return result;
}

/// std::string std::search
uint64_t std_string_find(const std::string& content,
                         const std::string& pattern) {
  uint64_t count = 0;
  std::string::const_iterator match = content.begin();
  while (true) {
    match = std::search(
        match, content.end(), pattern.begin(), pattern.end(),
        [](char c0, char c1) { return ::tolower(c0) == ::tolower(c1); });
    if (match == content.end()) {
      break;
    }
    ++count;
    match += pattern.size();
  }
  return count;
}

/// std::tolower + std::string::find
uint64_t std_lower_string_find(const std::string& content,
                               std::string pattern) {
  uint64_t count = 0;
  size_t shift = 0;
  std::string lower_content;
  lower_content.reserve(content.size());
  std::transform(content.begin(), content.end(), lower_content.begin(),
                 ::tolower);
  std::transform(pattern.begin(), pattern.end(), pattern.begin(), ::tolower);
  while ((shift = lower_content.find(pattern, shift)) != std::string::npos) {
    ++count;
    shift += pattern.size();
  }
  return count;
}

/// simd::tolower + simd::strstr
uint64_t simd_lower_strstr(const std::string& content, std::string pattern) {
  uint64_t result = 0;
  size_t shift = 0;
  std::string lower_content(content);
  xs::utils::str::simd::toLower(lower_content.data(), lower_content.size());
  xs::utils::str::simd::toLower(pattern.data(), pattern.size());
  while (shift < content.size()) {
    const char* match = xs::search::simd::strstr(
        lower_content.data() + shift, lower_content.size() - shift,
        pattern.data(), pattern.size());
    if (match == nullptr) {
      break;
    }
    result++;
    shift = (match - lower_content.data()) + pattern.size();
  }
  return result;
}

/// simd::strcasestr
uint64_t simd_strcasestr(const std::string& content,
                         const std::string& pattern) {
  uint64_t result = 0;
  size_t shift = 0;
  while (shift < content.size()) {
    const char* match = xs::search::simd::strcasestr(
        content.data() + shift, content.size() - shift, pattern.data(),
        pattern.size());
    if (match == nullptr) {
      break;
    }
    result++;
    shift = (match - content.data()) + pattern.size();
  }
  return result;
}

// -----------------------------------------------------------------------------

std::string read_content(const std::string& file) {
  std::ifstream stream(file);
  assert(stream);

  stream.seekg(0, std::ios::end);
  ssize_t file_size = stream.tellg();
  stream.seekg(0, std::ios::beg);

  std::string content;

  if (SIZE > file_size) {
    content.resize(file_size);
    stream.read(content.data(), file_size);
    while (content.size() < SIZE) {
      if (SIZE - content.size() > content.size()) {
        content += content;
      } else {
        content.append({content.data(), SIZE - content.size()});
      }
    }
  } else {
    content.resize(SIZE);
    stream.read(content.data(), SIZE);
    content.resize(stream.tellg());
  }

  assert(content.size() == SIZE);

  return content;
}

int main(int argc, char** argv) {
  std::string pattern(argv[1]);
  std::string file_path(argv[2]);

  auto content = read_content(file_path);
  std::stringstream info;
  info.imbue(std::locale(""));
  info << c_strcasestr(content, pattern) << " Matches in " << SIZE << " Bytes)";

  ankerl::nanobench::Bench bench;
  bench.title("Case insensitive substring search (" + info.str())
      .unit("byte")
      .batch(content.size())
      .relative(true);

  auto add_benchmark = [&bench]<typename Op>(const std::string& name,
                                             Op&& op) -> void {
    bench.run(name, std::forward<Op>(op));
  };

  add_benchmark("std::string::find", [&pattern, &content]() {
    auto res = std_string_find(content, pattern);
    ankerl::nanobench::doNotOptimizeAway(res);
  });
  add_benchmark("std::lower -> std::string::find", [&pattern, &content]() {
    auto res = std_lower_string_find(content, pattern);
    ankerl::nanobench::doNotOptimizeAway(res);
  });
  add_benchmark("re2", [&pattern, &content]() {
    auto res = re2_find(content, pattern);
    ankerl::nanobench::doNotOptimizeAway(res);
  });
  add_benchmark("simd::lower -> simd::strstr", [&pattern, &content]() {
    auto res = simd_lower_strstr(content, pattern);
    ankerl::nanobench::doNotOptimizeAway(res);
  });
  add_benchmark("simd::strcasestr", [&pattern, &content]() {
    auto res = simd_strcasestr(content, pattern);
    ankerl::nanobench::doNotOptimizeAway(res);
  });
  add_benchmark("std::lower -> std::strstr", [&pattern, &content]() {
    auto res = std_lower_strstr(content, pattern);
    ankerl::nanobench::doNotOptimizeAway(res);
  });
  add_benchmark("strcasestr", [&pattern, &content]() {
    auto res = c_strcasestr(content, pattern);
    ankerl::nanobench::doNotOptimizeAway(res);
  });
}