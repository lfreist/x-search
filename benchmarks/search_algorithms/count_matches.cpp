// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <nanobench.h>
#include <re2/re2.h>
#include <xsearch/string_search/simd_search.h>
#include <xsearch/utils/string_utils.h>

#include <cassert>
#include <fstream>
#include <functional>
#include <string>

#define SIZE 16777216

// Functions that are benchmarked ----------------------------------------------

/// Google RE2
uint64_t re2_find(const std::string& content, const std::string& pattern) {
  uint64_t count = 0;
  re2::RE2 re_pattern('(' + xs::utils::str::escaped(pattern) + ')');
  re2::StringPiece input(content.data(), content.size());
  re2::StringPiece match;
  while (re2::RE2::FindAndConsume(&input, re_pattern, &match)) {
    ++count;
  }
  return count;
}

/// std::strstr
uint64_t std_strstr(const std::string& content, const std::string& pattern) {
  int64_t result = 0;
  size_t shift = 0;
  while (true) {
    const char* match = std::strstr(content.c_str() + shift, pattern.c_str());
    if (match == nullptr) {
      break;
    }
    result++;
    shift = (match - content.data()) + pattern.size();
  }
  return result;
}

/// std::string::find
uint64_t std_string_find(const std::string& content,
                         const std::string& pattern) {
  uint64_t count = 0;
  size_t shift = 0;
  while ((shift = content.find(pattern, shift)) != std::string::npos) {
    ++count;
    shift += pattern.size();
  }
  return count;
}

/// simd::strstr
uint64_t simd_strstr(const std::string& content, const std::string& pattern) {
  uint64_t result = 0;
  size_t shift = 0;
  while (shift < content.size()) {
    const char* match =
        xs::search::simd::strstr(content.data() + shift, content.size() - shift,
                                 pattern.data(), pattern.size());
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
  info << std_strstr(content, pattern) << " Matches in " << SIZE << " Bytes)";

  ankerl::nanobench::Bench bench;
  bench.title("Case sensitive substring search (" + info.str())
      .unit("byte")
      .batch(content.size())
      .epochIterations(100)
      .relative(true)
      .warmup(10);

  auto add_benchmark = [&bench]<typename Op>(const std::string& name,
                                             Op&& op) -> void {
    bench.run(name, std::forward<Op>(op));
  };

  add_benchmark("std::string::find", [&pattern, &content]() {
    auto res = std_string_find(content, pattern);
    ankerl::nanobench::doNotOptimizeAway(res);
  });
  add_benchmark("std::strstr", [&pattern, &content]() {
    auto res = std_strstr(content, pattern);
    ankerl::nanobench::doNotOptimizeAway(res);
  });
  add_benchmark("re2", [&pattern, &content]() {
    auto res = re2_find(content, pattern);
    ankerl::nanobench::doNotOptimizeAway(res);
  });
  add_benchmark("simd::strstr", [&pattern, &content]() {
    auto res = simd_strstr(content, pattern);
    ankerl::nanobench::doNotOptimizeAway(res);
  });
}