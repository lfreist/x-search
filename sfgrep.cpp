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

// _____________________________________________________________________________
void output_helper(const std::string& pattern, xs::PartialResult& results,
                   bool regex, bool line_number, bool byte_offset,
                   bool only_matching, bool color) {
  for (size_t i = 0; i < results._byte_offsets.size(); ++i) {
    if (line_number) {
      if (color) {
        std::cout << GREEN << results._line_indices.at(i) + 1 << CYAN << ":"
                  << COLOR_RESET;
      } else {
        std::cout << results._line_indices.at(i) + 1 << ":";
      }
    }
    if (byte_offset) {
      if (color) {
        std::cout << GREEN << results._byte_offsets.at(i) << CYAN << ":"
                  << COLOR_RESET;
      } else {
        std::cout << results._byte_offsets.at(i) << ":";
      }
    }
    const std::string& line = results._lines.at(i);
    if (only_matching) {
      if (color) {
        std::cout << RED
                  << (regex ? get_regex_match_(line.data(), line.size(),
                                               re2::RE2("(" + pattern + ")"))
                            : pattern)
                  << COLOR_RESET << "\n";
      } else {
        std::cout << (regex ? get_regex_match_(line.data(), line.size(),
                                               re2::RE2("(" + pattern + ")"))
                            : pattern)
                  << "\n";
      }
    } else {
      if (color) {
        std::string match =
            regex ? get_regex_match_(line.data(), line.size(),
                                     re2::RE2("(" + pattern + ")"))
                  : pattern;
        // we need to find the offset within the line. Damn. Fix this another
        // day.
        const char* match_ = std::strstr(line.data(), match.data());
        if (match_ == nullptr) {
          std::cout << line;
          return;
        }
        size_t match_offset = match_ - line.data();
        std::cout << line.substr(0, match_offset) << RED << match << COLOR_RESET
                  << line.substr(match_offset + match.size());
      } else {
        std::cout << line;
      }
    }
  }
}

// _____________________________________________________________________________

class GrepResult : public xs::BaseResult<xs::PartialResult> {
 public:
  GrepResult(std::string pattern, bool regex, bool line_number,
             bool byte_offset, bool match_only, bool color = true)
      : _pattern(std::move(pattern)),
        _regex(regex),
        _color(color),
        _line_number(line_number),
        _byte_offset(byte_offset),
        _match_only(match_only) {}

  void addPartialResult(xs::PartialResult& part_res) override {
    INLINE_BENCHMARK_WALL_START("printing");
    if (part_res._index == _next_index) {
      output_helper(_pattern, part_res, _regex, _line_number, _byte_offset,
                    _match_only, _color);
      while (true) {
        _next_index++;
        auto search = _buffer.find(_next_index);
        if (search == _buffer.end()) {
          break;
        }
        output_helper(_pattern, search->second, _regex, _line_number,
                      _byte_offset, _match_only, _color);
        _buffer.erase(_next_index);
      }
    } else {
      _buffer.insert({part_res._index, {std::move(part_res)}});
    }
    INLINE_BENCHMARK_WALL_STOP("printing");
  }

  // only because we must implement it...
  xs::PartialResult& getResult() override { return _merged_result; }

 private:
  std::string _pattern;
  bool _regex;
  bool _color;
  bool _line_number;
  bool _byte_offset;
  bool _match_only;
  uint64_t _next_index = 0;
  std::unordered_map<uint64_t, xs::PartialResult> _buffer;
};

// -----------------------------------------------------------------------------

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
  std::vector<std::unique_ptr<
      xs::tasks::BaseSearcher<xs::DataChunk, xs::PartialResult>>>
      searchers;

  if (count) {
    // count set -> count results and output number in the end -----------------
    searchers.push_back(std::make_unique<xs::tasks::LineCounter>());
    auto extern_searcher = xs::ExternSearcher<>(
        pattern, num_threads, 2, std::move(reader), std::move(processors),
        std::move(searchers), xs::DefaultResult());
    extern_searcher.join();
    std::cout << extern_searcher.getResult().getResult()._count << std::endl;
    // -------------------------------------------------------------------------
  } else {
    // no count set -> run grep and output results
    // -------------------------------
    if ((byte_offset || line_number) && only_matching) {
      // -b -o || -n -o
      searchers.push_back(
          std::make_unique<xs::tasks::MatchBytePositionSearcher>());
    } else if ((byte_offset || line_number) && !only_matching) {
      // -b || -n
      searchers.push_back(
          std::make_unique<xs::tasks::LineBytePositionSearcher>());
    } else if (only_matching) {
      // -o
      searchers.push_back(
          std::make_unique<xs::tasks::MatchBytePositionSearcher>());
    } else {
      // no command line options
      searchers.push_back(
          std::make_unique<xs::tasks::LineBytePositionSearcher>());
    }
    if (line_number) {
      searchers.push_back(std::make_unique<xs::tasks::LineIndexSearcher>());
    }
    searchers.push_back(std::make_unique<xs::tasks::LinesSearcher>());
    auto extern_searcher =
        xs::ExternSearcher<xs::DataChunk, GrepResult, xs::PartialResult>(
            pattern, num_threads, 2, std::move(reader), std::move(processors),
            std::move(searchers),
            GrepResult(pattern, xs::utils::use_str_as_regex(pattern),
                       line_number, byte_offset, only_matching, !no_color));
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