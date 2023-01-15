// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/DataChunk.h>

namespace xs::collector {

class GrepOutput {
 public:
  GrepOutput(bool byte_offset, bool line_number, bool only_matching, bool count,
             bool color);
  ~GrepOutput() = default;

  void print(DataChunk* data);

  uint64_t getCount() const;

 private:
  bool _byte_offset;
  bool _line_number;
  bool _only_matching;
  bool _count;
  uint64_t _counter = 0;
  bool _color;
  // buffer and offset are used for outputting the retrieved results in the
  //  correct order.
  std::unordered_map<size_t, std::pair<SearchResults, size_t>> _buffer;
  size_t _offset = 0;
};

}  // namespace xs::collector