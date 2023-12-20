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

  xs::Searcher<xs::FileReader<xs::strtype>, xs::IndexSearcher<xs::strtype>, xs::Result<xs::PartRes1<size_t>>, xs::PartRes1<size_t>, void> searcher(
      xs::FileReader(file), xs::IndexSearcher(pattern), 1);
  auto res = searcher.execute<xs::execute::blocking>();

  while (searcher.running()) {
    std::cout << "running..." << std::endl;
    std::this_thread::sleep_for(std::chrono::nanoseconds (500));
  }

  for (auto& pr : res.get().get()) {
    for (auto offset : pr) {
      std::cout << offset << std::endl;
    }
  }

  return 0;
}