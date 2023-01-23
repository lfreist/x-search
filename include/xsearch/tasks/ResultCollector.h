// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

/**
 * DAS DING HIER KANN WEG!
 */

#pragma once

#include <xsearch/DataChunk.h>
#include <xsearch/ResultTypes.h>

namespace xs::tasks {

template <class RT = FullResult, class DT = DataChunk>
class BaseResultCollector {
 public:
  BaseResultCollector() = default;

  virtual void addPartialResult() = 0;
};

}  // namespace xs::tasks