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
  bool line_number = false;
  bool byte_offset = false;
  bool ignore_case = false;
  bool no_mmap = false;
  bool fixed_strings = false;
  size_t chunk_size = 16777216;
  bool count = false;
  std::string file_path;
  std::string meta_file_path;
  int num_threads = 0;
  int num_max_readers = 0;

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
  add("metafile,m", po::value<std::string>(&meta_file_path)->default_value(""),
      "metafile of the corresponding FILE");
  add("help,h", "prints this help message");
  add("version,V", "display version information and exit");
  add("threads,j",
      po::value<int>(&num_threads)->default_value(0)->implicit_value(0),
      "number of threads");
  add("max-readers", po::value<int>(&num_max_readers)->default_value(0),
      "number of concurrent reading tasks (default is number of threads");
  add("count,c", po::bool_switch(&count),
      "print only a count of selected lines");
  add("byte-offset,b", po::bool_switch(&byte_offset),
      "print the byte offset with output lines");
  add("line-number,n", po::bool_switch(&line_number),
      "print line number with output lines");
  add("ignore-case,i", po::bool_switch(&ignore_case)->default_value(false),
      "ignore case distinctions in patterns and data");
  add("chunk-size,s", po::value<size_t>(&chunk_size)->default_value(16777216),
      "min size of a single chunk that is read (default 16 MiB");
  add("fixed-strings,F", po::bool_switch(&fixed_strings)->default_value(false),
      "PATTERN is string (force no regex)");
  add("no-mmap", po::bool_switch(&no_mmap)->default_value(false),
      "do not use mmap but read data instead");
#ifdef BENCHMARK
  add("benchmark-file", po::value<std::string>(&benchmark_file),
      "set output file of benchmark measurements.");
  add("benchmark-format",
      po::value<std::string>(&benchmark_format)->default_value("json"),
      "specify the output format of benchmark measurements (plain, json, csv");
#endif

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

  // fix number of threads -----------------------------------------------------
  //  a) 0 -> 1
  //  b) < 0 -> number of threads available
  //  c) > number of threads available -> number of threads available
  int max_threads = static_cast<int>(std::thread::hardware_concurrency()) / 2;
  num_threads = num_threads <= 0 ? max_threads : num_threads;
  num_threads = num_threads > max_threads ? max_threads : num_threads;

  // fix number of max readers -------------------------------------------------
  //  a) <= 0 -> num_threads
  num_max_readers = num_max_readers <= 0 ? num_threads : num_max_readers;

  // ===== Setup xs::Executor for searching ====================================
  if (count) {
    auto searcher = xs::extern_search<xs::count_lines>(
        pattern, file_path, meta_file_path, ignore_case, num_threads,
        num_max_readers);
    searcher->join();
    std::cout << searcher->getResult()->size();
  } else {
    auto searcher =
        xs::extern_search<xs::lines>(pattern, file_path, meta_file_path,
                                     ignore_case, num_threads, num_max_readers);
    for (auto line : *searcher->getResult()) {
      std::cout << line << '\n';
    }
  }

  INLINE_BENCHMARK_WALL_STOP("total");
#ifdef BENCHMARK
  if (optionsMap.count("benchmark-file")) {
    std::ofstream out_stream(benchmark_file);
    out_stream << INLINE_BENCHMARK_REPORT(benchmark_format);
  } else {
    std::cerr << INLINE_BENCHMARK_REPORT(benchmark_format) << std::endl;
  }
#endif
  return 0;
}