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
struct GrepResultSettings {
  bool regex = false;
  bool index = false;
  bool color = true;
  bool only_matching = false;
  std::string pattern;
};

// _____________________________________________________________________________
class GrepResult
    : public xs::BaseResult<
          std::pair<std::vector<GrepPartialResult>, GrepResultSettings>> {
 public:
  GrepResult() = default;

  void add(std::pair<std::vector<GrepPartialResult>, GrepResultSettings>
               partial_result,
           uint64_t id) override {
    std::unique_lock lock(*this->_mutex);
    if (_current_index == id) {
      add(std::move(partial_result));
      _current_index++;
      // check if buffered results can be added now
      while (true) {
        auto search = _buffer.find(_current_index);
        if (search == _buffer.end()) {
          break;
        }
        add(std::move(search->second));
        _buffer.erase(_current_index);
        _current_index++;
      }
      // at least one partial_result was added -> notify
      this->_cv->notify_one();
    } else {
      // buffer the partial result
      _buffer.insert({id, std::move(partial_result)});
    }
  }

  size_t size() const override { return 0; }

 private:
  std::unordered_map<
      uint64_t, std::pair<std::vector<GrepPartialResult>, GrepResultSettings>>
      _buffer;
  uint64_t _current_index = 0;

  void add(std::pair<std::vector<GrepPartialResult>, GrepResultSettings>
               partial_result) override {
    auto& res = partial_result.first;
    auto& settings = partial_result.second;
    for (auto& r : res) {
      if (settings.index) {
        if (settings.color) {
          std::cout << GREEN << r.index << CYAN << ":" << COLOR_RESET;
        } else {
          std::cout << r.index << ":";
        }
      }
      if (settings.only_matching) {
        if (settings.color) {
          std::cout << RED << r.str << COLOR_RESET << '\n';
        } else {
          std::cout << r.str << '\n';
        }
      } else {
        if (settings.color) {
          // search for every occurrence of pattern within the string and
          // print it out colored while the rest is printed uncolored.
            size_t shift = 0;
            std::string match = settings.pattern;
            while (true) {
              size_t match_pos;
              if (settings.regex) {
                re2::RE2 re_pattern(settings.pattern);
                re2::StringPiece input(r.str.data() + shift, r.str.size() - shift);
                re2::StringPiece re_match;
                auto tmp = re2::RE2::PartialMatch(input, re_pattern, &re_match);
                if (tmp) {
                  match_pos = re_match.data() - input.data() + shift;
                  match = re_match.as_string();
                } else {
                  break;
                }
              } else {
                match_pos = r.str.find(settings.pattern, shift);
              }
              if (match_pos == std::string::npos) {
                break;
              }
              // print string part uncolored (eq. pythonic substr is
              //  str[shift:match_pos]) and pattern in RED
              std::cout << std::string(
                               r.str.begin() + static_cast<int64_t>(shift),
                               r.str.begin() + static_cast<int64_t>(match_pos))
                        << RED << match << COLOR_RESET;
              // start next search at new shift
              shift = match_pos + match.size();
            }
            // print rest of the string (eq. pythonic substr is str[shift:])
            std::cout << std::string(
                             r.str.begin() + static_cast<int64_t>(shift),
                             r.str.end())
                      << '\n';
        } else {
          std::cout << r.str << '\n';
        }
      }
    }
  }
};

// _____________________________________________________________________________
class GrepSearcher
    : public xs::tasks::BaseSearcher<
          xs::DataChunk,
          std::pair<std::vector<GrepPartialResult>, GrepResultSettings>> {
 public:
  GrepSearcher(bool byte_offset, bool line_number, bool only_matching,
               bool color)
      : _byte_offset(byte_offset),
        _line_number(line_number),
        _only_matching(only_matching),
        _color(color) {}

  std::pair<std::vector<GrepPartialResult>, GrepResultSettings> search(
      const std::string& pattern, xs::DataChunk* data) const override {
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

    std::pair<std::vector<GrepPartialResult>, GrepResultSettings> res = {
        {},
        {false, _line_number || _byte_offset, _color, _only_matching, pattern}};
    res.first.resize(byte_offsets.size());
    for (uint64_t i = 0; i < byte_offsets.size(); ++i) {
      res.first[i].index = _line_number ? line_numbers[i] : byte_offsets[i];
      res.first[i].str = _only_matching ? pattern : lines[i];
    }
    return res;
  }

  std::pair<std::vector<GrepPartialResult>, GrepResultSettings> search(
      re2::RE2* pattern, xs::DataChunk* data) const override {
    std::vector<uint64_t> byte_offsets =
        _only_matching
            ? xs::search::regex::global_byte_offsets_match(data, *pattern,
                                                           false)
            : xs::search::regex::global_byte_offsets_line(data, *pattern);
    std::vector<uint64_t> line_numbers;
    if (_line_number) {
      line_numbers = xs::map::bytes::to_line_indices(data, byte_offsets);
      std::transform(line_numbers.begin(), line_numbers.end(),
                     line_numbers.begin(), [](uint64_t li) { return li + 1; });
    }
    std::vector<std::string> lines;
    if (_only_matching) {
      lines.resize(byte_offsets.size());
      std::transform(byte_offsets.begin(), byte_offsets.end(), lines.begin(),
                     [data, pattern](uint64_t index) {
                       size_t local_byte_offset = index - data->getOffset();
                       return get_regex_match_(data->data() + local_byte_offset,
                                               data->size() - local_byte_offset,
                                               *pattern);
                     });
    } else {
      lines.resize(byte_offsets.size());
      std::transform(byte_offsets.begin(), byte_offsets.end(), lines.begin(),
                     [data](uint64_t index) {
                       return xs::map::byte::to_line(data, index);
                     });
    }

    std::pair<std::vector<GrepPartialResult>, GrepResultSettings> res = {
        {}, {true, _line_number || _byte_offset, _color, _only_matching, pattern->pattern()}};
    res.first.resize(byte_offsets.size());
    for (uint64_t i = 0; i < byte_offsets.size(); ++i) {
      res.first[i].index = _line_number ? line_numbers[i] : byte_offsets[i];
      // we have placed either the line or the regex match in here above!
      res.first[i].str = lines[i];
    }
    return res;
  }

 private:
  bool _byte_offset;
  bool _line_number;
  bool _only_matching;
  bool _color;
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
    auto searcher = std::make_unique<GrepSearcher>(byte_offset, line_number,
                                                   only_matching, !no_color);
    auto extern_searcher = xs::Searcher<
        xs::DataChunk, GrepResult,
        std::pair<std::vector<GrepPartialResult>, GrepResultSettings>>(
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