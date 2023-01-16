// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/DataChunk.h>
#include <xsearch/ResultTypes.h>
#include <xsearch/pipeline/result_collectors/MergeSearchResults.h>

#include <string>
#include <type_traits>
#include <vector>

namespace xs {

template <typename T>
class ExternSearcher {
 public:
  static Result<T> search(const std::string& file_path,
                          const std::string& meta_file_path,
                          const std::string& pattern, int num_threads = 1,
                          int max_readers = 1);
};

}  // namespace xs