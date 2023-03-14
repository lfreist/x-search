// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/xsearch.h>

#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;

int main(int argc, char** argv) {
  INLINE_BENCHMARK_WALL_START(_, "total");
#ifdef BENCHMARK
  std::string benchmark_file;
  std::string benchmark_format;
#endif
  std::string pattern;
  std::string file_path;
  bool ignore_case = false;
  bool count = false;
  int num_threads = 2;

  po::options_description options("Options for example/grep");
  po::positional_options_description positional_options;

  // wrappers for adding command line arguments --------------------------------
  auto add_positional =
      [&positional_options]<typename... Args>(Args&&... args) {
        positional_options.add(std::forward<Args>(args)...);
      };
  auto add = [&options]<typename... Args>(Args&&... args) {
    options.add_options()(std::forward<Args>(args)...);
  };

  // defining possible command line arguments ----------------------------------
  add_positional("PATTERN", 1);
  add_positional("FILE", 1);
  add("PATTERN", po::value<std::string>(&pattern)->required(),
      "search pattern");
  add("FILE", po::value<std::string>(&file_path)->default_value(""),
      "input file, stdin if '-' or empty");
  add("help,h", "prints this help message");
  add("count,c", po::bool_switch(&count),
      "print only a count of selected lines");
  add("ignore-case,i", po::bool_switch(&ignore_case)->default_value(false),
      "ignore case distinctions in patterns and data");

  // parse command line options ------------------------------------------------
  po::variables_map optionsMap;
  try {
    po::store(po::command_line_parser(argc, argv)
                  .options(options)
                  .positional(positional_options)
                  .run(),
              optionsMap);
    if (optionsMap.count("help")) {
      std::cout << options << std::endl;
      return 0;
    }
    po::notify(optionsMap);
  } catch (const std::exception& e) {
    std::cerr << "Error in command line argument: " << e.what() << std::endl;
    std::cerr << options << std::endl;
    return 1;
  }

  // ===== Setup xs::Executor for searching ====================================
  if (count) {
    auto searcher = xs::extern_search<xs::count_lines>(
        pattern, file_path, ignore_case, num_threads);
    searcher->join();
    std::cout << searcher->getResult()->size() << std::endl;
  } else {
    auto searcher =
        xs::extern_search<xs::lines>(pattern, file_path,
                                     ignore_case, num_threads);
    for (auto const& line : *searcher->getResult()) {
      std::cout << line << '\n';
    }
  }

  return 0;
}