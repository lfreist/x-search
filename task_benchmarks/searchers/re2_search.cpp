// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/string_search/search_wrappers.h>
#include <xsearch/utils/string_utils.h>
#include <xsearch/utils/InlineBench.h>

#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>

namespace po = boost::program_options;

uint64_t count(const char* data, size_t size, const re2::RE2& pattern) {
  uint64_t counter = 0;
  re2::StringPiece input(data, size);
  re2::StringPiece match;
  while (re2::RE2::PartialMatch(input, pattern, &match)) {
    size_t shift = match.data() - input.data();
    counter++;
    shift += match.size();
    input.remove_prefix(shift);
  }
  return counter;
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


  re2::RE2::Options re2_options;
  re2_options.set_case_sensitive(!case_insensitive);
  re2_options.set_posix_syntax(true);

  std::unique_ptr<re2::RE2> regex_pattern;
  if (literal) {
    std::string escaped_pattern("(");
    for (char c : pattern) {
      if (c == '\\' || c == '.' || c == '[' || c == ']' || c == '(' || c == ')' || c == '{' || c == '}' || c == '|' || c == '*' || c == '+' || c == '?' || c == '^' || c == '$') {
        escaped_pattern += '\\';
      }
      escaped_pattern += c;
    }
    escaped_pattern.push_back(')');
    regex_pattern = std::make_unique<re2::RE2>(escaped_pattern, re2_options);
  } else {
    regex_pattern = std::make_unique<re2::RE2>(std::string('(' + pattern + ')'), re2_options);
  }

  INLINE_BENCHMARK_WALL_START(_, "search");
  auto c = count(content.data(), content.size(), *regex_pattern);
  INLINE_BENCHMARK_WALL_STOP("search");

  std::cout << c << std::endl;
  std::cerr << INLINE_BENCHMARK_REPORT("json") << std::endl;
  return 0;
}