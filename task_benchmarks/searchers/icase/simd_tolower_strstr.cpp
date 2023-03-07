// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/string_search/simd_search.h>
#include <xsearch/utils/string_utils.h>

#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

uint64_t count_matches(char* data, size_t size, char* pattern,
                       size_t pattern_size) {
  uint64_t count = 0;
  size_t shift = 0;
  while (shift < size) {
    char* match = xs::search::simd::strstr(data + shift, size - shift, pattern,
                                           pattern_size);
    if (match == nullptr) {
      break;
    }
    count++;
    shift = (match - data) + pattern_size;
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

  // ----- transform pattern and data to lower case ----------------------------
  std::vector<char> content_lower(content.begin(), content.end());
  xs::utils::str::simd::toLower(pattern.data(), pattern.size());
  xs::utils::str::simd::toLower(content_lower.data(), content_lower.size());
  // ----- count matches using simd::strcasestr --------------------------------
  auto num_matches = count_matches(content_lower.data(), content_lower.size(),
                                   pattern.data(), pattern.size());
  // ----- search done ---------------------------------------------------------

  std::cout << num_matches << std::endl;

  return 0;
}