// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <lz4.h>

#include <vector>

namespace xs::utils::compression {

class LZ4 {
 public:
  /**
   * @brief: Compress the given byte array and return the result
   *
   * @param src
   * @param numBytes
   * @param acceleration default is 1. 0 is set to 1. Each increase boosts
   * compression speed by ~3%.
   * @return compressed data
   */
  static std::vector<char> compress(const char* src, int numBytes,
                                    int acceleration = 1) {
    std::vector<char> result(LZ4_compressBound(numBytes));
    auto compressedSize =
        LZ4_compress_fast(src, result.data(), numBytes,
                          static_cast<int>(result.size()), acceleration);
    result.resize(compressedSize);
    return result;
  }

  /**
   * @brief: Decompress the given byte array, assuming that the size of the
   * decompressed data is known and return the result.
   *
   * @tparam T
   * @param src
   * @param numBytes
   * @param originalSize
   * @return uncompressed data
   */
  template <typename T>
  static std::vector<T> decompress(const char* src, size_t numBytes,
                                   size_t originalSize) {
    std::vector<T> result(originalSize);
    auto decompressedSize =
        LZ4_decompress_safe(src, result.data(), numBytes, result.size());
    result.resize(decompressedSize);
    return result;
  }

  /**
   * @brief: Decompress the given byte array to a given buffer, assuming that
   * the buffers capacity is sufficient.
   *
   * @tparam T
   * @param src
   * @param numBytes
   * @param buffer
   * @param bufferCapacity
   * @return size of uncompressed data
   */
  template <typename T>
  static size_t decompressToBuffer(const char* src, size_t numBytes, T* buffer,
                                   size_t bufferCapacity) {
    auto decompressedSize =
        LZ4_decompress_safe(src, buffer, numBytes, bufferCapacity);
    return decompressedSize;
  }
};

}  // namespace xs::utils::compression