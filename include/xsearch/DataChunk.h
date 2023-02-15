// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/MetaFile.h>
#include <xsearch/utils/UninitializedAllocator.h>

#include <string>
#include <vector>

#include "gtest/gtest.h"

namespace xs {

// using strtype = std::vector<char, xs::utils::just_allocator<char>>;
using strtype = char*;

/**
 * SFString is a string type object, internally holding a std::basic_string
 * initialized with an allocator that does not initialize the reserved memory.
 *
 * Later, a string view like SFString might be implemented for operating on data
 * that are owned by another process/object or whatever.
 */
class DataChunk {
  FRIEND_TEST(DataChunk, constructor);
  FRIEND_TEST(DataChunk, MoveConstructor);
  FRIEND_TEST(DataChunk, size);
  FRIEND_TEST(DataChunk, resize);
  FRIEND_TEST(DataChunk, push_back);
  FRIEND_TEST(DataChunk, assign);
  FRIEND_TEST(DataChunk, data);

 public:
  DataChunk() = default;
  DataChunk(char* data, size_t size, ChunkMetaData meta_data);
  explicit DataChunk(strtype data, size_t size);
  explicit DataChunk(ChunkMetaData meta_data);

  ~DataChunk();

  // delete copy constructor
  DataChunk(const DataChunk&) = delete;
  DataChunk& operator=(const DataChunk&) = delete;
  // move constructor
  DataChunk(DataChunk&& chunk) noexcept;
  DataChunk& operator=(DataChunk&&) noexcept = default;

  ChunkMetaData& getMetaData();
  [[nodiscard]] const ChunkMetaData& getMetaData() const;

  /**
   * wrapper method for this->_data.data()
   */
  [[nodiscard]] char* data() const;

  /**
   * wrapper method for this->_data.size()
   */
  [[nodiscard]] size_t size() const;

  /**
   * wrapper method for this->_data.resize()
   */
  void resize(size_t size);

  /**
   * wrapper method for this->_data.assign()
   */
  void assign(std::string data);

  // TODO: make private again!
  char* _data = nullptr;
  void set_size(size_t size) { _size = size; };
  void set_mmap() { _mmap = true; };
  void set_mmap_offset(size_t offset) { _mmap_offset = offset; }

 private:
  // indicates if _data must be destructed or if it is owned by another process
  bool _data_moved = false;
  size_t _size = 0;
  bool _mmap = false;
  size_t _mmap_offset = 0;
  ChunkMetaData _meta_data{0, 0, 0, 0, 0, {}};
};

}  // namespace xs
