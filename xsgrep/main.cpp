// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/xsearch.h>

#include <boost/program_options.hpp>
#include <iostream>

#include "./components.h"

namespace po = boost::program_options;

/**
 * Arguments used by XS grep. Inspired by GNU grep.
 */
struct GrepArgs {
  std::string pattern;
  std::string file_path;
  std::string meta_file_path;
  int num_threads = 0;
  int num_max_readers = 0;
  bool count = false;
  bool byte_offset = false;
  bool line_number = false;
  bool only_matching = false;
  bool color = false;
  bool ignore_case = false;
  size_t chunk_size = 16777216;
  bool fixed_strings = false;
  bool no_mmap = false;
};

int main(int argc, char** argv) {
  INLINE_BENCHMARK_WALL_START(_, "total");
  GrepArgs args;
#ifdef BENCHMARK
  std::string benchmark_file;
  std::string benchmark_format;
#endif

  po::options_description options("Options for ExternStringFinderMain");
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
  add_positional("METAFILE", 1);
  add("PATTERN", po::value<std::string>(&args.pattern)->required(),
      "search pattern");
  add("FILE", po::value<std::string>(&args.file_path)->default_value(""),
      "input file, stdin if '-' or empty");
  add("METAFILE",
      po::value<std::string>(&args.meta_file_path)->default_value(""),
      "metafile of the corresponding FILE");
  add("help,h", "prints this help message");
  add("version,V", "display version information and exit");
  add("threads,j",
      po::value<int>(&args.num_threads)->default_value(0)->implicit_value(0),
      "number of threads");
  add("max-readers", po::value<int>(&args.num_max_readers)->default_value(0),
      "number of concurrent reading tasks (default is number of threads");
  add("count,c", po::bool_switch(&args.count),
      "print only a count of selected lines");
  add("byte-offset,b", po::bool_switch(&args.byte_offset),
      "print the byte offset with output lines");
  add("line-number,n", po::bool_switch(&args.line_number),
      "print line number with output lines");
  add("only-matching,o", po::bool_switch(&args.only_matching),
      "show only nonempty parts of lines that match");
  add("ignore-case,i", po::bool_switch(&args.ignore_case)->default_value(false),
      "ignore case distinctions in patterns and data");
  add("chunk-size,s",
      po::value<size_t>(&args.chunk_size)->default_value(16777216),
      "min size of a single chunk that is read (default 16 MiB");
  add("fixed-strings,F",
      po::bool_switch(&args.fixed_strings)->default_value(false),
      "PATTERN is string (force no regex)");
  add("no-mmap", po::bool_switch(&args.no_mmap)->default_value(false),
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
  args.color = isatty(STDOUT_FILENO);

  // fix number of threads -----------------------------------------------------
  //  a) 0 -> 1
  //  b) < 0 -> number of threads available
  //  c) > number of threads available -> number of threads available
  int max_threads = static_cast<int>(std::thread::hardware_concurrency()) / 2;
  args.num_threads = args.num_threads <= 0 ? max_threads : args.num_threads;
  args.num_threads =
      args.num_threads > max_threads ? max_threads : args.num_threads;

  // fix number of max readers -------------------------------------------------
  //  a) <= 0 -> num_threads
  args.num_max_readers =
      args.num_max_readers <= 0 ? args.num_threads : args.num_max_readers;

  // ===== Setup xs::Executor for searching ====================================

  std::vector<std::unique_ptr<xs::tasks::BaseInplaceProcessor<xs::DataChunk>>>
      processors;

  // check for compression and add decompression task --------------------------
  if (args.meta_file_path.empty()) {
    if (args.line_number) {
      // this task creates new line mapping data necessary for searching line
      // numbers
      processors.push_back(std::make_unique<xs::tasks::NewLineSearcher>());
    }
  } else {
    xs::MetaFile metaFile(args.meta_file_path, std::ios::in);
    switch (metaFile.getCompressionType()) {
      case xs::CompressionType::LZ4:
        processors.push_back(std::make_unique<xs::tasks::LZ4Decompressor>());
        break;
      case xs::CompressionType::ZSTD:
        processors.emplace_back(
            std::make_unique<xs::tasks::ZSTDDecompressor>());
        break;
      default:
        break;
    }
  }

  // check if data should be transformed to lower case (--ignore-case) ---------
  if (args.ignore_case) {
    std::transform(args.pattern.begin(), args.pattern.end(),
                   args.pattern.begin(), [](int c) { return ::tolower(c); });
  }

  // set reader ----------------------------------------------------------------
  std::unique_ptr<xs::tasks::BaseDataProvider<xs::DataChunk>> reader;
  if (args.file_path.empty() || args.file_path == "-") {
    // read stdin
    // for porting to windows, we need to open 'CON' instead of '/dev/stdin'...
    reader = std::make_unique<xs::tasks::FileBlockReader>("/dev/stdin",
                                                          args.chunk_size);
  } else {
    if (args.meta_file_path.empty()) {
      // no meta file provided
      if (args.no_mmap ||
          args.chunk_size < static_cast<uint64_t>(sysconf(_SC_PAGE_SIZE))) {
        // don't read using mmap
        reader = std::make_unique<xs::tasks::FileBlockReader>(args.file_path,
                                                              args.chunk_size);
      } else {  // read using mmap
        reader = std::make_unique<xs::tasks::FileBlockReaderMMAP>(
            args.file_path, args.chunk_size);
      }
    } else {  // meta file provided
      if (args.no_mmap) {
        // don't read using mmap
        reader = std::make_unique<xs::tasks::FileBlockMetaReader>(
            args.file_path, args.meta_file_path);
      } else {  // read using mmap
        reader = std::make_unique<xs::tasks::FileBlockMetaReaderMMAP>(
            args.file_path, args.meta_file_path);
      }
    }
  }

  // set searchers -------------------------------------------------------------
  if (args.count) {
    // count set -> count results and output number in the end -----------------
    auto searcher = std::make_unique<xs::tasks::LineCounter>(
        args.pattern,
        xs::utils::use_str_as_regex(args.pattern) && !args.fixed_strings,
        args.ignore_case);
    auto extern_searcher =
        xs::Executor<xs::DataChunk, xs::CountResult, uint64_t>(
            args.num_threads, args.num_max_readers, std::move(reader),
            std::move(processors), std::move(searcher));
    extern_searcher.join();
    std::cout << extern_searcher.getResult()->size() << std::endl;
    // -------------------------------------------------------------------------
  } else {
    // no count set -> run grep and output results
    auto searcher = std::make_unique<GrepSearcher>(
        args.pattern,
        xs::utils::use_str_as_regex(args.pattern) && !args.fixed_strings,
        args.ignore_case, args.line_number, args.only_matching);
    auto extern_searcher =
        xs::Executor<xs::DataChunk, GrepResult, std::vector<GrepPartialResult>,
                     std::string, bool, bool, bool, bool, bool>(
            args.num_threads, args.num_max_readers, std::move(reader),
            std::move(processors), std::move(searcher),
            std::string(args.pattern),
            !args.fixed_strings && xs::utils::use_str_as_regex(args.pattern),
            bool(args.byte_offset), bool(args.line_number),
            bool(args.only_matching), bool(args.color));
    extern_searcher.join();
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