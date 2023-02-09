// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/xsearch.h>

#include <boost/program_options.hpp>
#include <iostream>
#include <numeric>
#include <string>
#include <thread>
#include <vector>

namespace po = boost::program_options;

// ===== Grep output Result ====================================================
// _____ Output Formatting _____________________________________________________
// _____________________________________________________________________________
std::string get_regex_match_(const char* data, size_t size,
                             const re2::RE2& pattern) {
  re2::StringPiece input(data, size);
  re2::StringPiece match;
  re2::RE2::PartialMatch(input, pattern, &match);
  return match.as_string();
}

struct GrepPartialResult {
  uint64_t index = 0;
  std::string str;

  bool operator<(const GrepPartialResult& other) const {
    return index < other.index;
  }
};

// _____________________________________________________________________________
class GrepResult : public xs::ContainerResult<GrepPartialResult> {
 public:
  GrepResult() = default;
};

// _____________________________________________________________________________
void output_helper(const std::string& pattern, GrepResult& results, bool regex,
                   bool index, bool only_matching, bool color) {
  for (uint64_t i = 0; i < results.size(); ++i) {
    if (index) {
      if (color) {
        std::cout << GREEN << results[i].index << CYAN << ":" << COLOR_RESET;
      } else {
        std::cout << results[i].index << ":";
      }
    }
    // TODO: add colored output
    std::cout << results[i].str << std::endl;
  }
}

// _____________________________________________________________________________
class GrepSearcher
    : public xs::tasks::BaseSearcher<xs::DataChunk,
                                     std::vector<GrepPartialResult>> {
 public:
  GrepSearcher(bool line_number, bool only_matching)
      : _line_number(line_number), _only_matching(only_matching) {}

  std::vector<GrepPartialResult> search(const std::string& pattern,
                                        xs::DataChunk* data) const override {
    std::vector<uint64_t> byte_offsets =
        _only_matching
            ? xs::search::global_byte_offsets_match(data, pattern, false)
            : xs::search::global_byte_offsets_line(data, pattern);
    std::vector<uint64_t> line_numbers;
    if (_line_number) {
      line_numbers = xs::map::bytes::to_line_indices(data, byte_offsets);
      std::transform(line_numbers.begin(), line_numbers.end(),
                     line_numbers.begin(), [](uint64_t li) { return li + 1; });
    }
    std::vector<std::string> lines;
    if (!_only_matching) {
      lines.resize(byte_offsets.size());
      std::transform(byte_offsets.begin(), byte_offsets.end(), lines.begin(),
                     [data](uint64_t index) {
                       return xs::map::byte::to_line(data, index);
                     });
    }

    std::vector<GrepPartialResult> res(byte_offsets.size());
    for (uint64_t i = 0; i < byte_offsets.size(); ++i) {
      res[i].index = _line_number ? line_numbers[i] : byte_offsets[i];
      res[i].str = _only_matching ? pattern : lines[i];
    }
    return res;
  }

  std::vector<GrepPartialResult> search(re2::RE2* pattern,
                                        xs::DataChunk* data) const override {
    return {};
  }

 private:
  bool _line_number;
  bool _only_matching;
};

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

  // creating TaskManager with its tasks and run it ----------------------------
  std::vector<std::unique_ptr<xs::tasks::BaseProcessor<xs::DataChunk>>>
      processors;

  // check for compression and add decompression task --------------------------
  xs::MetaFile metaFile(meta_file_path, std::ios::in);
  switch (metaFile.getCompressionType()) {
    case xs::CompressionType::LZ4:
      processors.push_back(std::make_unique<xs::tasks::LZ4Decompressor>());
      break;
    case xs::CompressionType::ZSTD:
      processors.emplace_back(std::make_unique<xs::tasks::ZSTDDecompressor>());
      break;
    default:
      break;
  }
  // ---------------------------------------------------------------------------
  auto reader =
      std::make_unique<xs::tasks::ExternBlockReader>(file_path, meta_file_path);

  if (count) {
    // count set -> count results and output number in the end -----------------
    auto searcher = std::make_unique<xs::tasks::LineCounter>();
    INLINE_BENCHMARK_WALL_START("total");
    auto extern_searcher =
        xs::Searcher<xs::DataChunk, xs::CountResult, uint64_t>(
            pattern, num_threads, num_threads, std::move(reader),
            std::move(processors), std::move(searcher));
    extern_searcher.join();
    INLINE_BENCHMARK_WALL_STOP("total");
    std::cout << extern_searcher.getResult()->size() << std::endl;
    // -------------------------------------------------------------------------
  } else {
    // no count set -> run grep and output results
    auto searcher = std::make_unique<GrepSearcher>(line_number, only_matching);
    auto extern_searcher =
        xs::Searcher<xs::DataChunk, GrepResult, std::vector<GrepPartialResult>>(
            pattern, num_threads, num_threads, std::move(reader),
            std::move(processors), std::move(searcher));
    extern_searcher.join();
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

// int main() {
//   return 0;
// }