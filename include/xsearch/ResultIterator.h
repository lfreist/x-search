// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <type_traits>
#include <xsearch/Searcher.h>

namespace xs {

template <class DataT, class ResultT, class PartResT>
class ResultIterator {
 public:
  ResultIterator(Searcher<DataT, ResultT, PartResT>& searcher, uint64_t index)
      : _searcher(searcher), _index(index) {}

  PartResT& operator*() {
    std::unique_lock lock(_searcher._results);
  }

 private:
  ResultT& _searcher;
  uint64_t _index;
};

}