/**
 * Copyright 2023, Leon Freist (https://github.com/lfreist)
 * Author: Leon Freist <freist.leon@gmail.com>
 *
 * This file is part of x-search.
 */

#pragma once

#include <xsearch/concepts.h>
#include <xsearch/types.h>

#include <fstream>
#include <istream>
#include <optional>
#include <string>

namespace xs {

template <DefaultDataC T = xs::strtype>
class FileReader {
 public:
  FileReader(std::string file_path, size_t chunk_size = 524288)
      : _file_path(std::move(file_path)), _fstream(_file_path), _chunk_size(chunk_size) {}
  ~FileReader() { _fstream.close(); }

  FileReader(FileReader&&) = default;
  FileReader& operator=(FileReader&&) = default;

  std::optional<T> read() {
    if (_fstream.eof()) {
      return {};
    }
    T data;
    data.resize(_chunk_size);
    _fstream.read(data.data(), _chunk_size);
    data.resize(_fstream.gcount());
    return std::make_optional(std::move(data));
  }

 private:
  size_t _chunk_size;
  std::string _file_path;
  std::ifstream _fstream;
};

}  // namespace xs