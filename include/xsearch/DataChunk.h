// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/MetaFile.h>
#include <xsearch/utils/UninitializedAllocator.h>

#include <string>
#include <vector>

#include "gtest/gtest.h"

namespace xs {

struct SearchResults {
  std::string pattern;
  bool regex;
  size_t offset;
  std::optional<std::vector<size_t>> _local_byte_offsets{};
  std::optional<std::vector<size_t>> _global_line_indices{};
  std::optional<std::vector<std::string>> _matching_lines{};
  uint64_t _match_count = 0;
};

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

 public:
  DataChunk() = default;
  explicit DataChunk(
      strtype data, uint64_t offset = 0,
      std::vector<ByteToNewLineMappingInfo> byte_nl_mapping = {});
  DataChunk(strtype data, uint64_t originalSize, uint64_t offset,
            std::vector<ByteToNewLineMappingInfo> byte_nl_mapping = {});
  explicit DataChunk(
      uint64_t data_size, uint64_t offset = 0,
      std::vector<ByteToNewLineMappingInfo> byte_nl_mapping = {});
  DataChunk(uint64_t data_size, uint64_t original_size, uint64_t offset = 0,
            std::vector<ByteToNewLineMappingInfo> byte_nl_mapping = {});

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
  [[nodiscard]] uint64_t getOriginalSize() const;
  void setOffset(uint64_t offset);
  void setOriginalSize(uint64_t original_size);

  /**
   * Access to the internally hold std::basic_string using a pointer.
   * @return Pointer to _content
   */
  char* data();

  [[nodiscard]] const char* data() const;

  [[nodiscard]] size_t size() const;

  void resize(size_t size);

  [[nodiscard]] const std::vector<ByteToNewLineMappingInfo>& getNewLineIndices()
      const;

  std::vector<ByteToNewLineMappingInfo> moveNewLineIndices();

  void push_back(char c);

  void reserve(size_t size);

  void assign(std::string data);

  void pop_back();

 private:
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
