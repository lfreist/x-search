// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/DataChunk.h>
#include <xsearch/ResultTypes.h>

#include <string>
#include <type_traits>
#include <vector>

namespace xs {

template <typename T, typename Enable = void>
class ExternSearcher {};

template <typename T>
class ExternSearcher<
    T, typename std::enable_if<std::is_same<T, restype::count>::value>::type> {
 public:
  static size_t search(const std::string& file_path,
                       const std::string& meta_file_path,
                       const std::string& pattern, int num_threads = 1,
                       int max_readers = 1);
};

template <typename T>
class ExternSearcher<T,
                     typename std::enable_if<
                         std::is_same<T, restype::byte_positions>::value ||
                         std::is_same<T, restype::line_numbers>::value ||
                         std::is_same<T, restype::line_indices>::value>::type> {
 public:
  static std::vector<size_t> search(const std::string& file_path,
                                    const std::string& meta_file_path,
                                    const std::string& pattern,
                                    int num_threads = 1, int max_readers = 1);
};

template <typename T>
class ExternSearcher<
    T, typename std::enable_if<std::is_same<T, restype::lines>::value>::type> {
 public:
  static std::vector<std::string> search(const std::string& file_path,
                                         const std::string& meta_file_path,
                                         const std::string& pattern,
                                         int num_threads = 1,
                                         int max_readers = 1);
};

template <typename T>
class ExternSearcher<
    T, typename std::enable_if<std::is_same<T, restype::full>::value>::type> {
 public:
  static std::vector<SearchResults> search(const std::string& file_path,
                                           const std::string& meta_file_path,
                                           const std::string& pattern,
                                           int num_threads = 1,
                                           int max_readers = 1);
};

}  // namespace xs