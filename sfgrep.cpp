// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/xsearch.h>

#include <boost/program_options.hpp>
#include <iostream>
#include <numeric>
#include <regex>
#include <string>
#include <thread>
#include <vector>

namespace po = boost::program_options;

int main(int argc, char** argv) {
  std::string pattern;
  std::string file_path;
  std::string meta_file_path;
  int num_threads;
  bool count;
  bool byte_offset;
  bool line_number;
  bool only_matching;
  bool no_color;
  std::string benchmarkFile;

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
  // ---------------------------------------------------------------------------

  // defining possible command line arguments ----------------------------------
  //  most of them are equivalent to GNU grep
  add_positional("PATTERN", 1);
  add_positional("FILE", 1);
  add_positional("METAFILE", 1);
  add("PATTERN", po::value<std::string>(&pattern)->required(),
      "search pattern");
  add("FILE", po::value<std::string>(&file_path)->required(), "input file");
  add("METAFILE", po::value<std::string>(&meta_file_path)->default_value(""),
      "metafile of the corresponding FILE");
  add("help,h", "Prints this help message");
  add("threads,j", po::value<int>(&num_threads)->default_value(1),
      "number of threads");
  add("count,c", po::bool_switch(&count),
      "print only a count of selected lines");
  add("byte-offset,b", po::bool_switch(&byte_offset),
      "print the byte offset with output lines");
  add("line-number,n", po::bool_switch(&line_number),
      "print line number with output lines");
  add("only-matching,o", po::bool_switch(&only_matching),
      "show only nonempty parts of lines that match");
  add("no-color", po::bool_switch(&no_color), "do not use colored output.");
  add("benchmark", po::value<std::string>(&benchmarkFile),
      "Use code benchmark.");
  // ---------------------------------------------------------------------------

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
  // ---------------------------------------------------------------------------

  // fix number of threads, if it was chosen: ----------------------------------
  //  a) as 0 -> 1
  //  b) < 0 -> number of threads available
  //  c) > number of threads available -> number of threads available
  int max_threads = static_cast<int>(std::thread::hardware_concurrency());
  num_threads = num_threads < 0 ? max_threads : num_threads;
  num_threads = num_threads > max_threads ? max_threads : num_threads;
  num_threads = num_threads == 0 ? 1 : num_threads;
  // ---------------------------------------------------------------------------

  // check if provided pattern is regex ----------------------------------------
  bool regex = xs::utils::use_str_as_regex(pattern);
  // ---------------------------------------------------------------------------

  // creating TaskManager with its tasks and run it ----------------------------
  std::vector<xs::pipeline::ProcessingTask> processors;

  // check for compression and add decompression task --------------------------
  xs::MetaFile metaFile(meta_file_path, std::ios::in);
  switch (metaFile.getCompressionType()) {
    case xs::CompressionType::LZ4:
      processors.emplace_back(
          [](auto&& PH1) { xs::processors::decompress::using_lz4(PH1); });
      break;
    case xs::CompressionType::ZSTD:
      processors.emplace_back(
          [](auto&& PH1) { xs::processors::decompress::using_zstd(PH1); });
      break;
    default:
      break;
  }
  // ---------------------------------------------------------------------------
  xs::reader::BlockReader reader(std::move(file_path),
                                 std::move(meta_file_path), 10);
  xs::searcher::Searcher searcher(std::move(pattern), regex);
  xs::collector::GrepOutput grep(byte_offset, line_number, only_matching, count,
                                 !no_color);

  // reader task
  xs::pipeline::ProducerTask rTask([&reader] { return reader.read(); }, 2);

  if (count) {
    // count set -> count results and output number in the end -----------------
    processors.emplace_back([&searcher](auto&& PH1) {
      searcher.count(std::forward<decltype(PH1)>(PH1));
    });
    // output task
    xs::pipeline::CollectorTask<uint64_t> oTask(
        [&grep](auto&& PH1) { grep.print(std::forward<decltype(PH1)>(PH1)); },
        [&grep]() -> uint64_t { return grep.getCount(); });
    xs::pipeline::TaskManager<uint64_t> task_manager(
        std::move(rTask), std::move(processors), std::move(oTask));
    task_manager.execute(num_threads);
    task_manager.join();
    // -------------------------------------------------------------------------
  } else {
    // no count set -> run grep and output results
    // -------------------------------
    if ((byte_offset || line_number) && only_matching) {
      processors.emplace_back([&searcher](auto&& PH1) {
        searcher.byte_offsets_match(std::forward<decltype(PH1)>(PH1), false);
      });
    } else if ((byte_offset || line_number) && !only_matching) {
      processors.emplace_back([&searcher](auto&& PH1) {
        searcher.byte_offsets_line(std::forward<decltype(PH1)>(PH1));
      });
    } else {
      processors.emplace_back([&searcher](auto&& PH1) {
        searcher.byte_offsets_line(std::forward<decltype(PH1)>(PH1));
      });
    }
    if (line_number) {
      processors.emplace_back([&searcher](auto&& PH1) {
        searcher.line_indices(std::forward<decltype(PH1)>(PH1));
      });
    }
    processors.emplace_back([&searcher](auto&& PH1) {
      searcher.line(std::forward<decltype(PH1)>(PH1));
    });
    // output task
    xs::pipeline::CollectorTask<void> oTask(
        [&grep](auto&& PH1) { grep.print(std::forward<decltype(PH1)>(PH1)); },
        []() -> void { return; });

    xs::pipeline::TaskManager<void> task_manager(
        std::move(rTask), std::move(processors), std::move(oTask));
    task_manager.execute(num_threads);
    task_manager.join();
  }
  // ---------------------------------------------------------------------------

#ifdef BENCHMARK
  if (optionsMap.count("benchmark")) {
    std::ofstream benchmarkOutput(benchmarkFile);
    benchmarkOutput << INLINE_BENCHMARK_REPORT("json");
  } else {
    std::cout << INLINE_BENCHMARK_REPORT("plain") << std::endl;
  }
#else
  if (optionsMap.count("benchmark")) {
    std::cout << "Benchmark was not performed! Please compile with "
                 "'-DBENCHMARK' flag.";
  }
#endif

  return 0;
}