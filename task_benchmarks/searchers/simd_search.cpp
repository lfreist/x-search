// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/xsearch.h>

#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>

namespace po = boost::program_options;

uint64_t count(char* data, size_t size, std::string pattern) {
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
  std::string file;
  std::string pattern;
  bool case_insensitive = false;
  bool literal = false;

  po::options_description options("Options for re2_search");
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
  add("FILE", po::value<std::string>(&file)->required(), "search pattern");
  add("case-insensitive,i", po::bool_switch(&case_insensitive));
  add("literal,l", po::bool_switch(&literal));

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

  std::ifstream stream(file);
  if (!stream.is_open()) {
    std::cerr << "Error reading file '" << file << "'\n";
    return 2;
  }

  std::ostringstream ss;
  ss << stream.rdbuf();
  std::string content = ss.str();

  INLINE_BENCHMARK_WALL_START(_, "search");
  if (case_insensitive) {
    std::transform(pattern.begin(), pattern.end(), pattern.begin(), ::tolower);
    xs::utils::str::simd::toLower(content.data(), content.size());
  }
  auto c = count(content.data(), content.size(), pattern);
  INLINE_BENCHMARK_WALL_STOP("search");

  std::cout << c << std::endl;

  std::cerr << INLINE_BENCHMARK_REPORT("json") << std::endl;
  return 0;
}