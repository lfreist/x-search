// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/xsearch.h>

class MetaDataCreator : public xs::tasks::BaseSearcher<xs::DataChunk, xs::ChunkMetaData> {
 public:
  MetaDataCreator() = default;
  xs::ChunkMetaData search(const std::string& pattern, xs::DataChunk* data) const override;

 private:
  xs::ChunkMetaData search(re2::RE2* pattern, xs::DataChunk* data) const override;
};

class MetaDataWriter : public xs::BaseResult<xs::ChunkMetaData> {
 public:
  explicit MetaDataWriter(const std::string& meta_file_path);

  void add(xs::ChunkMetaData meta_data, uint64_t id) override;

 private:
  void add(xs::ChunkMetaData meta_data) override;

  std::unordered_map<uint64_t, xs::ChunkMetaData> _buffer;
  uint64_t _current_index = 0;
  xs::MetaFile _meta_file;
};