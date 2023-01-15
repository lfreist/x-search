// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/DataChunk.h>
#include <xsearch/MetaFile.h>

#include <functional>
#include <memory>
#include <mutex>
#include <string>

namespace xs::reader {

class BlockReader {
 public:
  explicit BlockReader(std::string file_path, std::string meta_file,
                       int blocks_per_read = 10);
  ~BlockReader() = default;

  std::vector<DataChunk> read();

 private:
  int _chunks_per_read;
  std::string _file_path;
  std::string _meta_file_path;
  MetaFile _meta_file;
};

}  // namespace xs::reader