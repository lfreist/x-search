/**
 * Copyright 2023, Leon Freist (https://github.com/lfreist)
 * Author: Leon Freist <freist.leon@gmail.com>
 *
 * This file is part of x-search.
 */

#include <xsearch/Searcher.h>
#include <xsearch/tasks/readers.h>
#include <xsearch/tasks/searchers.h>
#include <xsearch/ResultTypes.h>

#include <iostream>
#include <string>
#include <fstream>


int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "Usage: ./exe <pattern> <file>" << std::endl;
    return 1;
  }
  std::string pattern(argv[1]);
  std::string file(argv[2]);

  xs::Searcher<xs::FileReader<xs::strtype>, xs::LineSearcher<xs::strtype>, xs::Result<xs::PartRes1<std::string>>, xs::PartRes1<std::string>, void> searcher(
      xs::FileReader(file), xs::LineSearcher(pattern), 1);
  auto res = searcher.execute<xs::execute::live>();

  for (const auto& pr : res.get()) {
    for (const auto& offset : pr) {
      std::cout << offset << std::endl;
    }
  }

  return 0;
}