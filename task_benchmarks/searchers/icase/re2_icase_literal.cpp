// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <re2/re2.h>

#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

uint64_t count_matches(char* data, size_t size, const re2::RE2& pattern) {
  uint64_t count = 0;
  re2::StringPiece input(data, size);
  re2::StringPiece match;
  while (re2::RE2::FindAndConsume(&input, pattern, &match)) {
    ++count;
  }
  return count;
}

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "Usage: ./exe <pattern> <file_path>" << std::endl;
  }
  std::string pattern(argv[1]);
  std::string file_path(argv[2]);

  std::ifstream stream(file_path);
  assert(stream);

  stream.seekg(0, std::ios::end);
  size_t file_size = stream.tellg();
  stream.seekg(0, std::ios::beg);

  std::string content;
  content.resize(file_size);
  stream.read(content.data(), file_size);

  re2::RE2::Options options;
  options.set_case_sensitive(false);
  re2::RE2 re_pattern('(' + pattern + ')', options);

  // ----- count matches using simd::strcasestr --------------------------------
  auto num_matches = count_matches(content.data(), content.size(), re_pattern);
  // ----- search done ---------------------------------------------------------

  std::cout << num_matches << std::endl;

  return 0;
}