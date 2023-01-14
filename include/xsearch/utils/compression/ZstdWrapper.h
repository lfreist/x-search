// Copyright 2021, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Johannes Kalmbach <johannes.kalmbach@gmail.com>

// Edited by Leon Freist, 2022:
//  - removed some AD_UTILS dependencies
//  - changed namespace
//  - renaming

#pragma once

#include <zstd.h>

#include <string>
#include <vector>

namespace xs::utils::compression {

class ZSTD {
 public:
  // Compress the given byte array and return the result;
  static std::vector<char> compress(void* src, size_t src_size,
                                    int compression_level = 3) {
    std::vector<char> result(ZSTD_compressBound(src_size));
    auto compressed_size = ZSTD_compress(result.data(), result.size(), src,
                                         src_size, compression_level);
    if (ZSTD_isError(compressed_size)) {
      throw std::runtime_error("ZSTD compression error: " +
                               std::to_string(compressed_size));
    }
    result.resize(compressed_size);
    return result;
  }

  // Decompress the given byte array, assuming that the size of the decompressed
  // data is known.
  template <typename T>
  static std::vector<T> decompress(void* src, size_t src_size,
                                   size_t uncompressed_size) {
    uncompressed_size *= sizeof(T);
    std::vector<T> result(uncompressed_size / sizeof(T));
    auto decompressed_size =
        ZSTD_decompress(result.data(), uncompressed_size, src, src_size);
    if (ZSTD_isError(decompressed_size)) {
      throw std::runtime_error("ZSTD decompression error: " +
                               std::to_string(decompressed_size));
    }
    return result;
  }

  // Decompress the given byte array to the given buffer of the given size,
  // returning the number of bytes of the decompressed data.
  template <typename T>
  static size_t decompressToBuffer(const char* src, size_t src_size, T* buffer,
                                   size_t buffer_capacity) {
    auto decompressed_size = ZSTD_decompress(buffer, buffer_capacity,
                                             const_cast<char*>(src), src_size);
    if (ZSTD_isError(decompressed_size)) {
      throw std::runtime_error("ZSTD decompression error: " +
                               std::to_string(decompressed_size));
    }
    return decompressed_size;
  }
};

}  // namespace xs::utils::compression