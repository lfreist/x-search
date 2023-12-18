/**
 * Copyright 2023, Leon Freist (https://github.com/lfreist)
 * Author: Leon Freist <freist.leon@gmail.com>
 *
 * This file is part of x-search.
 */

#include <xsearch/Executor.h>
#include <xsearch/string_search/simd_search.h>

#include <iostream>
#include <string>

struct Reader {
  explicit Reader(const std::string& file) {
    _stream = std::ifstream(file);
    if (!_stream) {
      std::cerr << "Error opening file " << file << std::endl;
    }
  }

  std::optional<xs::DataChunk> read() {
    if (_stream.eof()) {
      return {};
    }
    xs::DataChunk chunk(1024);
    _stream.read(chunk.data(), 1024);
    return {chunk};
  }

  std::ifstream _stream;
};

struct PartialResult {
  explicit PartialResult(std::string pattern) : _pattern(std::move(pattern)) {}

  std::string _pattern;
  std::vector<size_t> offset;
};

struct Result {
  Result() = default;

  void add(PartialResult pr) { _results.push_back(std::move(pr)); }

  std::vector<PartialResult>& get() { return _results; }

  std::vector<PartialResult> _results;
};

struct Searcher {
  explicit Searcher(std::string pattern) : _pattern(std::move(pattern)) {}

  std::optional<PartialResult> search(xs::DataChunk* data) {
    if (data == nullptr) {
      return {};
    }
    PartialResult pr(_pattern);
    size_t shift = 0;
    while (true) {
      int64_t match = xs::search::simd::findNext(
          _pattern.data(), _pattern.size(), data->data(), data->size(), shift);
      if (match < 0) {
        break;
      }
      pr.offset.push_back(match);
      shift += match;
      shift += _pattern.size();
    }
    if (pr.offset.empty()) {
      return {};
    }
    return {pr};
  }

  std::string _pattern;
};

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "Usage: ./exe <pattern> <file>" << std::endl;
    return 1;
  }
  std::string pattern(argv[1]);
  std::string file(argv[2]);

  xs::Searcher<Reader, Searcher, Result, PartialResult, void> searcher(
      Reader(file), Searcher(pattern), 1);
  auto res = searcher.execute<xs::execute::async>();

  while (searcher.running()) {
    std::cout << "running..." << std::endl;
    std::this_thread::sleep_for(std::chrono::nanoseconds (1));
  }

  for (auto& pr : res.get()._results) {
    for (auto offset : pr.offset) {
      std::cout << offset << std::endl;
    }
  }

  return 0;
}