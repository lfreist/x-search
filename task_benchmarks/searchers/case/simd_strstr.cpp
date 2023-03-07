// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/string_search/simd_search.h>

#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

uint64_t count_matches(char* data, size_t size, const std::string& pattern) {
  uint64_t result = 0;
  size_t shift = 0;
  while (shift < size) {
    char* match = xs::search::simd::strstr(data + shift, size - shift,
                                           pattern.data(), pattern.size());
    if (match == nullptr) {
      break;
    }
    result++;
    shift = (match - data) + pattern.size();
  }
  return result;
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

  // ----- count matches -------------------------------------------------------
  auto num_matches = count_matches(content.data(), content.size(), pattern);
  // ----- search done ---------------------------------------------------------

  std::cout << num_matches << std::endl;

  return 0;
}