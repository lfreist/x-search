// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/ExternSearcher.h>
#include <xsearch/pipeline/Task.h>
#include <xsearch/pipeline/TaskManager.h>
#include <xsearch/pipeline/processors/decompressors.h>
#include <xsearch/pipeline/reader/BlockReader.h>
#include <xsearch/pipeline/result_collectors/MergeSearchResults.h>
#include <xsearch/pipeline/searchers/Searchers.h>
#include <xsearch/utils/utils.h>

#include <string>

namespace xs {

template <>
size_t ExternSearcher<restype::count>::search(const std::string& file_path,
                                              const std::string& meta_file_path,
                                              const std::string& pattern,
                                              int num_threads,
                                              int max_readers) {
  xs::tasks::reader::BlockReader reader(file_path, meta_file_path);
  xs::tasks::Searcher searcher(pattern, xs::utils::use_str_as_regex(pattern));
}

template <>
std::vector<size_t> ExternSearcher<restype::byte_positions>::search(
    const std::string& file_path, const std::string& meta_file_path,
    const std::string& pattern, int num_threads, int max_readers) {
  return {};
}

template <>
std::vector<size_t> ExternSearcher<restype::line_numbers>::search(
    const std::string& file_path, const std::string& meta_file_path,
    const std::string& pattern, int num_threads, int max_readers) {
  return {};
}

template <>
std::vector<size_t> ExternSearcher<restype::line_indices>::search(
    const std::string& file_path, const std::string& meta_file_path,
    const std::string& pattern, int num_threads, int max_readers) {
  return {};
}

template <>
std::vector<std::string> ExternSearcher<restype::lines>::search(
    const std::string& file_path, const std::string& meta_file_path,
    const std::string& pattern, int num_threads, int max_readers) {
  return {};
}

template <>
std::vector<SearchResults> ExternSearcher<restype::full>::search(
    const std::string& file_path, const std::string& meta_file_path,
    const std::string& pattern, int num_threads, int max_readers) {
  return {};
}

}  // namespace xs