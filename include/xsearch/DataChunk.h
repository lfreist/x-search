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
  explicit DataChunk(strtype data, uint64_t offset = 0,
                     std::vector<ByteToNewLineMappingInfo> byte_nl_mapping = {},
                     size_t index = 0);
  DataChunk(strtype data, uint64_t originalSize, uint64_t offset,
            std::vector<ByteToNewLineMappingInfo> byte_nl_mapping = {},
            size_t index = 0);
  explicit DataChunk(uint64_t data_size, uint64_t offset = 0,
                     std::vector<ByteToNewLineMappingInfo> byte_nl_mapping = {},
                     size_t index = 0);
  DataChunk(uint64_t data_size, uint64_t original_size, uint64_t offset = 0,
            std::vector<ByteToNewLineMappingInfo> byte_nl_mapping = {},
            size_t index = 0);

  // delete copy constructor
  DataChunk(const DataChunk&) = delete;
  DataChunk& operator=(const DataChunk&) = delete;
  // move constructor
  DataChunk(DataChunk&&) noexcept = default;
  DataChunk& operator=(DataChunk&&) noexcept = default;

  /**
   * Const access to the internally hold std::basic_string
   * @return const reference to _content
   */
  [[nodiscard]] const strtype& str() const;

  [[nodiscard]] uint64_t getOffset() const;
  void setOffset(uint64_t offset);
  [[nodiscard]] uint64_t getOriginalSize() const;
  void setOriginalSize(uint64_t original_size);
  [[nodiscard]] size_t getIndex() const;

  /**
   * wrapper method for this->_content.data()
   */
  char* data();

  /**
   * wrapper method for this->_content.data()
   */
  [[nodiscard]] const char* data() const;

  /**
   * wrapper method for this->_content.size()
   */
  [[nodiscard]] size_t size() const;

  /**
   * wrapper method for this->_content.resize()
   */
  void resize(size_t size);

  [[nodiscard]] const std::vector<ByteToNewLineMappingInfo>& getMappingData()
      const;

  /**
   * Move mapping data.
   * !! MARK: mappiing data are no available for this DataChunk instance
   *     afterwards !!
   * @return
   */
  std::vector<ByteToNewLineMappingInfo> moveMappingData();

  /**
   * wrapper method for this->_content.push_back()
   */
  void push_back(char c);

  /**
   * wrapper method for this->_content.reserve()
   */
  void reserve(size_t size);

  /**
   * wrapper method for this->_content.assign()
   */
  void assign(std::string data);

 private:
  // index of the DataChunk (e.g. if a file is read in chunks, _index is the
  //  index of the chunk corresponding to the file)
  size_t _index = 0;
  // actual text like content
  strtype _content;
  // indicating position of _content's first byte relative to start of all data
  uint64_t _offset = 0;
  // if _content is compressed, the size of its uncompressed data must be known
  uint64_t _originalSize = 0;

  // vector of a pair of new line indices and byte positions relative to start
  // of all data (if meta file is provided)
  std::vector<ByteToNewLineMappingInfo> _byte_to_nl_mapping_data{};
};

}  // namespace xs
