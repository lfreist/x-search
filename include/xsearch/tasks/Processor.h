// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/DataChunk.h>

namespace xs::tasks {

template <class DataT = DataChunk>
class BaseProcessor {
 public:
  BaseProcessor() = default;
  virtual ~BaseProcessor() = default;

  virtual void process(DataT* data) = 0;
};

class LZ4Decompressor : public BaseProcessor<DataChunk> {
 public:
  LZ4Decompressor() = default;

  void process(DataChunk* data) override;
};

class ZSTDDecompressor : public BaseProcessor<DataChunk> {
 public:
  ZSTDDecompressor() = default;

  void process(DataChunk* data) override;
};

}  // namespace xs::tasks