// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/MetaFile.h>
#include <xsearch/utils/UninitializedAllocator.h>

#include <string>
#include <vector>

#include "gtest/gtest.h"

namespace xs {

using strtype = std::vector<char, xs::utils::just_allocator<char>>;

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
  DataChunk(strtype data, ChunkMetaData meta_data);
  explicit DataChunk(strtype data);
  explicit DataChunk(ChunkMetaData meta_data);

  // delete copy constructor
  DataChunk(const DataChunk&) = delete;
  DataChunk& operator=(const DataChunk&) = delete;
  // move constructor
  DataChunk(DataChunk&&) noexcept = default;
  DataChunk& operator=(DataChunk&&) noexcept = default;

  /**
   * Access to the internally hold std::basic_string
   * @return reference to _data
   */
  strtype& getData();
  [[nodiscard]] const strtype& getData() const;

  ChunkMetaData& getMetaData();
  [[nodiscard]] const ChunkMetaData& getMetaData() const;

  /**
   * wrapper method for this->_data.data()
   */
  char* data();

  /**
   * wrapper method for this->_data.data()
   */
  [[nodiscard]] const char* data() const;

  /**
   * wrapper method for this->_data.size()
   */
  [[nodiscard]] size_t size() const;

  /**
   * wrapper method for this->_data.resize()
   */
  void resize(size_t size);

  /**
   * wrapper method for this->_data.push_back()
   */
  void push_back(char c);

  /**
   * wrapper method for this->_data.reserve()
   */
  void reserve(size_t size);

  /**
   * wrapper method for this->_data.assign()
   */
  void assign(std::string data);

 private:
  ChunkMetaData _meta_data{0, 0, 0, 0, 0, {}};
  strtype _data;
};

}  // namespace xs
